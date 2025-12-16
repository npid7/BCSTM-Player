#include <arpa/inet.h>

#include <bcstm/bcstmv2.hpp>

void D7::BCSTM2::LoadFile(const std::string& path) {
  Stop();
  pFile.open(path, std::ios::in | std::ios::binary);
  if (!pFile) {
    throw std::runtime_error("BCSTM [ERROR]: Unable to load File!");
  }
  pBigEndian = false;
  is_little_endian = true;  // default to true

  auto magic = Read<u32>();
  if ((pBigEndian = (Read<u16>() != 0xFEFF))) {
    magic = htonl(magic);
  }
  if (magic != 0x4D545343) {  // CSTM
    pFile.close();
    throw std::runtime_error("BCSTM [ERROR]: File is invalid!");
  }
  pFile.seekg(0x10);
  auto sbc = Read<u16>();
  Read<u16>();

  for (unsigned short i = 0; i < sbc; i++) {
    auto sec = Read<u16>();
    Read<u16>();  // padding
    auto off = Read<u32>();
    Read<u32>();  // size
    if (sec == InfoBlock)
      info_offset = off;
    else if (sec == DataBlock)
      data_offset = off;
  }
  if (!info_offset || !data_offset) {
    throw std::runtime_error("BCSTM [ERROR]: Data/Info Section not found");
  }

  pFile.seekg((info_offset + 0x20));
  if (Read<u8>() != 2) {
    throw std::runtime_error(
        "BCSTM [ERROR]: Only DSP ADPCM Format is supported!");
  }
  is_looping = Read<u8>();
  channel_count = Read<u8>();
  /*if (channel_count > 2) {
    throw std::runtime_error("BCSTM [ERROR]: File has not 2 channels!");
  }*/
  pFile.seekg((info_offset + 0x24));
  sample_rate = Read<u32>();
  auto _loop_pos = Read<u32>();
  auto _loop_end = Read<u32>();
  num_blocks = Read<u32>();
  block_size = Read<u32>();
  block_samples = Read<u32>();
  Read<u32>();  // last block used bytes
  last_block_samples = Read<u32>();
  last_block_size = Read<u32>();

  loop_start = _loop_pos / block_samples;
  loop_end =
      (_loop_end % block_samples ? num_blocks : _loop_end / block_samples);

  while (Read<u32>() != ChannelInfo) {
    // Find Channel Info
  }
  file_advance(Read<u32>() + channel_count * 8 - 12);
  // get adpcm data
  for (unsigned int i = 0; i < channel_count; i++) {
    pFile.read(reinterpret_cast<char*>(adpcm_coefs[i]),
               sizeof(unsigned short) * 16);
    // beginning context
    pFile.read(reinterpret_cast<char*>(&adpcm_data[i][0]),
               sizeof(ndspAdpcmData));
    // loop context
    pFile.read(reinterpret_cast<char*>(&adpcm_data[i][1]),
               sizeof(ndspAdpcmData));
    // skip padding
    Read<u16>();
  }

  pFile.seekg((data_offset + 0x20));
  is_loaded = true;
}

void D7::BCSTM2::Stream() { this->stream(); }

void D7::BCSTM2::Play() {
  if (is_paused) {
    for (unsigned int i = 0; i < channel_count; i++) {
      ndspChnSetPaused(channel[i], false);
    }
    is_paused = false;
    return;
  }
  if (is_streaming) return;
  for (unsigned int i = 0; i < channel_count; i++) {
    {
      channel[i] = 0;
      while (channel[i] < 24 && ((active_channels >> channel[i]) & 1)) {
        channel[i]++;
      }
      if (channel[i] == 24) {
        std::cout << "BCSTM [ERROR]: Current chennel equals 24!" << std::endl;
        return;
      }
      active_channels |= 1 << channel[i];
      ndspChnWaveBufClear(channel[i]);
    }
    static float mix[12];
    ndspChnSetFormat(channel[i],
                     NDSP_FORMAT_ADPCM | NDSP_3D_SURROUND_PREPROCESSED);
    ndspChnSetRate(channel[i], sample_rate);

    /** Make sure they are 0 */
    for (int j = 0; j < 12; j++) {
      mix[j] = 0.f;
    }
    /** Leave 1 and 2 Channels as they are */
    if (channel_count == 1) {
      mix[0] = mix[1] = 0.5f;
    } else if (channel_count == 2) {
      if (i == 0) {
        mix[0] = 0.8f;
        mix[2] = 0.2f;
      } else {
        mix[1] = 0.8f;
        mix[3] = 0.2f;
      }
      /** Dont know if this is the finished code here */
    } else if (channel_count == 4) {
      mix[i] = 0.8f;
      if (i > 0) mix[i - 1] += 0.2f;
      if (i < 3) mix[i + 1] += 0.2f;
    } else if (channel_count == 6) {
      mix[i] = 0.7f;
      if (i > 0) mix[i - 1] += 0.15f;
      if (i < 5) mix[i + 1] += 0.15f;
    } else if (channel_count == 8) {
      mix[i] = 0.6f;
      if (i > 0) mix[i - 1] += 0.1f;
      if (i < 7) mix[i + 1] += 0.1f;
    }

    ndspChnSetMix(channel[i], mix);
    ndspChnSetAdpcmCoefs(channel[i], adpcm_coefs[i]);

    for (unsigned int j = 0; j < buffer_count; j++) {
      memset(&wave_buf[i][j], 0, sizeof(ndspWaveBuf));
      wave_buf[i][j].status = NDSP_WBUF_DONE;
      buffer_data[i][j].resize(block_size);
    }
  }
  is_streaming = true;
}

void D7::BCSTM2::Pause() {
  if (!is_streaming) return;
  is_paused = true;
  for (unsigned int i = 0; i < channel_count; i++) {
    ndspChnSetPaused(channel[i], true);
  }
}

void D7::BCSTM2::Stop() {
  if (pFile) pFile.close();
  /** Move this loop up (logical issue in < v2.0.0) */
  for (unsigned int i = 0; i < channel_count; i++) {
    ndspChnWaveBufClear(channel[i]);
    ndspChnReset(channel[i]);
    active_channels &= ~(1 << channel[i]);
  }
  channel_count = 0;
  sample_rate = 0;
  loop_start = 0;
  loop_end = 0;
  num_blocks = 0;
  block_size = 0;
  block_samples = 0;
  last_block_size = 0;
  last_block_samples = 0;
  current_block = 0;
  info_offset = 0;
  data_offset = 0;
  active_channels = 0;
  is_paused = false;
  is_looping = false;
  is_loaded = false;
  m_is_ending = false;
  if (!is_streaming) return;
  is_streaming = false;
}

void D7::BCSTM2::stream() {
  current_time = svcGetSystemTick();
  if (current_time - last_time >= 100000000 && is_loaded) {
    if (m_is_ending) {
      bool all_done = true;
      for (PD::u32 buf = 0; buf < buffer_count; buf++) {
        for (PD::u8 chn = 0; chn < channel_count; chn++) {
          if (wave_buf[chn][buf].status != NDSP_WBUF_DONE) {
            all_done = false;
            break;
          }
        }
      }
      if (all_done) {
        Stop();
      }
      return;
    }
    if (!is_streaming) return;
    if (!is_paused) fill_buffers();
    last_time = current_time;
  }
}

void D7::BCSTM2::fill_buffers() {
  for (unsigned int bufIndex = 0; bufIndex < buffer_count; ++bufIndex) {
    /*if (wave_buf[0][bufIndex].status != NDSP_WBUF_DONE) continue;
    if (channel_count == 2 && wave_buf[1][bufIndex].status != NDSP_WBUF_DONE)
      continue;*/

    bool all_ready = true;
    for (unsigned int ch = 0; ch < channel_count; ++ch) {
      if (wave_buf[ch][bufIndex].status != NDSP_WBUF_DONE) {
        all_ready = false;
        break;
      }
    }
    if (!all_ready) continue;

    if (is_looping && current_block == loop_end) {
      current_block = loop_start;
      pFile.seekg(
          (data_offset + 0x20 + block_size * channel_count * loop_start));
    }
    if (!is_looping && current_block == loop_end) {
      m_is_ending = true;
      return;
    }

    for (unsigned int channelIndex = 0; channelIndex < channel_count;
         ++channelIndex) {
      ndspWaveBuf* buf = &wave_buf[channelIndex][bufIndex];

      memset(buf, 0, sizeof(ndspWaveBuf));
      buf->data_adpcm = buffer_data[channelIndex][bufIndex].data();
      pFile.read(
          reinterpret_cast<char*>(buf->data_adpcm),
          (current_block == num_blocks - 1) ? last_block_size : block_size);
      DSP_FlushDataCache(buf->data_adpcm, block_size);

      if (current_block == 0)
        buf->adpcm_data = &adpcm_data[channelIndex][0];
      else if (current_block == loop_start)
        buf->adpcm_data = &adpcm_data[channelIndex][1];

      if (current_block == num_blocks - 1)
        buf->nsamples = last_block_samples;
      else
        buf->nsamples = block_samples;

      ndspChnWaveBufAdd(channel[channelIndex], buf);
    }

    current_block++;
  }
}

bool D7::BCSTM2::file_advance(unsigned long long bytes) {
  size_t seek_pos = (size_t)pFile.tellg() + bytes;
  pFile.seekg(seek_pos);
  return (!pFile.fail() && pFile.tellg() == seek_pos);
}