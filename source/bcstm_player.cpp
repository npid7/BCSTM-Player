#include <bcstm_player.hpp>
#include <cstring> /** std::memset :( */

namespace D7 {
void BcstmPlayer::LoadFile(const std::string& path) {
  CleanUp();
  try {
    pCurrentFile.LoadFile(path);
  } catch (const std::exception& e) {
    throw std::runtime_error(e.what());
  }
  /** Resize the Player Internal Data holders */
  /** Using this allows to not have to much memory allocated */
  pChannels.Resize(pCurrentFile.GetNumChannels());
  pWaveBuf.Resize(pCurrentFile.GetNumChannels());
  // pBufferData.Resize(pCurrentFile.GetNumChannels());
  for (PD::u32 i = 0; i < pCurrentFile.GetNumChannels(); i++) {
    pWaveBuf[i].Resize(BufferCount);
    // pBufferData[i].Resize(BufferCount);
  }
  pIsLoaded = true;
}

void BcstmPlayer::Play() {
  if (pIsPaused) {
    for (PD::u32 i = 0; i < pCurrentFile.GetNumChannels(); i++) {
      ndspChnSetPaused(pChannels[i], false);
    }
    pIsPaused = false;
    return;
  }
  if (pIsStreaming) {
    return;
  }
  for (PD::u32 i = 0; i < pCurrentFile.GetNumChannels(); i++) {
    {
      pChannels[i] = 0;
      while (pChannels[i] < 24 && ((pActiveChannels >> pChannels[i]) & 1)) {
        pChannels[i]++;
      }
      if (pChannels[i] == 24) {
        throw std::range_error(
            "BCSTM Player: Out of Range (channel == 24) detected!");
      }
      pActiveChannels |= 1 << pChannels[i];
      ndspChnWaveBufClear(pChannels[i]);
    }
    static float mix[12];
    ndspChnSetFormat(pChannels[i],
                     NDSP_FORMAT_ADPCM | NDSP_3D_SURROUND_PREPROCESSED);
    ndspChnSetRate(pChannels[i], pCurrentFile.GetSampleRate());

    /** Make sure they are 0 */
    for (int j = 0; j < 12; j++) {
      mix[j] = 0.f;
    }
    /** Leave 1 and 2 Channels as they are */
    if (pCurrentFile.GetNumChannels() == 1) {
      mix[0] = mix[1] = 0.5f;
    } else if (pCurrentFile.GetNumChannels() == 2) {
      if (i == 0) {
        mix[0] = 0.8f;
        mix[2] = 0.2f;
      } else {
        mix[1] = 0.8f;
        mix[3] = 0.2f;
      }
      /** Dont know if this is the finished code here */
    } else if (pCurrentFile.GetNumChannels() == 4) {
      mix[i] = 0.8f;
      if (i > 0) mix[i - 1] += 0.2f;
      if (i < 3) mix[i + 1] += 0.2f;
    } else if (pCurrentFile.GetNumChannels() == 6) {
      mix[i] = 0.7f;
      if (i > 0) mix[i - 1] += 0.15f;
      if (i < 5) mix[i + 1] += 0.15f;
    } else if (pCurrentFile.GetNumChannels() == 8) {
      mix[i] = 0.6f;
      if (i > 0) mix[i - 1] += 0.1f;
      if (i < 7) mix[i + 1] += 0.1f;
    }

    ndspChnSetMix(pChannels[i], mix);
    ndspChnSetAdpcmCoefs(pChannels[i],
                         pCurrentFile.pDSP_ADPCM_Info[i].Param.Coefficients);

    for (unsigned int j = 0; j < BufferCount; j++) {
      /** still Prefer fill_n over memset */
      std::memset(&pWaveBuf[i][j], 0, sizeof(ndspWaveBuf));
      pWaveBuf[i][j].status = NDSP_WBUF_DONE;
      pBufferData[i][j].Resize(pCurrentFile.GetBlockSize());
    }
  }
  pIsStreaming = true;
}

void BcstmPlayer::Pause() {
  if (!pIsStreaming) {
    return;
  }
  pIsPaused = true;
  for (PD::u32 i = 0; i < pCurrentFile.GetNumChannels(); i++) {
    ndspChnSetPaused(pChannels[i], true);
  }
}

void BcstmPlayer::Stop() {
  if (!pIsStreaming) {
    return;
  }
  for (unsigned int i = 0; i < pCurrentFile.GetNumChannels(); i++) {
    ndspChnWaveBufClear(pChannels[i]);
    pActiveChannels &= ~(1 << pChannels[i]);
  }
  pIsStreaming = false;
}

void BcstmPlayer::Stream() {
  pCurrentTime = svcGetSystemTick();
  if (pCurrentTime - pLastTime >= 100000000 && pIsLoaded) {
    if (!pIsStreaming) return;
    if (!pIsPaused) pFillBuffers();
    pLastTime = pCurrentTime;
  }
}

void BcstmPlayer::pFillBuffers() {
  for (PD::u32 buf_idx = 0; buf_idx < BufferCount; buf_idx++) {
    bool all_ready = true;
    for (PD::u32 ch = 0; ch < pCurrentFile.GetNumChannels(); ch++) {
      if (pWaveBuf[ch][buf_idx].status != NDSP_WBUF_DONE) {
        all_ready = false;
        break;
      }
    }
    if (!all_ready) continue;

    if (pCurrentFile.IsLooping() &&
        pCurrentBlock == pCurrentFile.GetLoopEnd()) {
      pCurrentBlock = pCurrentFile.GetLoopStart();
      pCurrentFile.ReadGotoBeginning(true);
    }
    if (!pCurrentFile.IsLooping() &&
        pCurrentBlock == pCurrentFile.GetLoopEnd()) {
      this->Stop();
    }

    for (PD::u32 chn_idx = 0; chn_idx < pCurrentFile.GetNumChannels();
         ++chn_idx) {
      ndspWaveBuf* buf = &pWaveBuf[chn_idx][buf_idx];

      memset(buf, 0, sizeof(ndspWaveBuf));
      buf->data_adpcm = pBufferData[chn_idx][buf_idx].Data();
      pCurrentFile.ReadBlock(pCurrentBlock, (PD::u8*)buf->adpcm_data);
      DSP_FlushDataCache(buf->data_adpcm, pCurrentFile.GetBlockSize());

      if (pCurrentBlock == 0) {
        buf->adpcm_data =
            (ndspAdpcmData*)&pCurrentFile.pDSP_ADPCM_Info[chn_idx].Context;
      } else if (pCurrentBlock == pCurrentFile.GetLoopStart()) {
        buf->adpcm_data =
            (ndspAdpcmData*)&pCurrentFile.pDSP_ADPCM_Info[chn_idx].LoopContext;
      }

      if (pCurrentBlock == pCurrentFile.GetNumBlocks() - 1) {
        buf->nsamples = pCurrentFile.GetLastBlockSamples();
      } else {
        buf->nsamples = pCurrentFile.GetBlockSamples();
      }

      ndspChnWaveBufAdd(pChannels[chn_idx], buf);
    }
    pCurrentBlock++;
  }
}

void BcstmPlayer::CleanUp() {
  Stop();
  if (pIsLoaded) {
    try {
      pCurrentFile.CleanUp();
    } catch (const std::exception& e) {
      throw std::runtime_error(e.what());
    }
  }
  pIsLoaded = false;
  pIsStreaming = false;
  pIsPaused = false;
  pActiveChannels = 0;
  pChannels.Clear();
  pWaveBuf.Clear();
}
}  // namespace D7