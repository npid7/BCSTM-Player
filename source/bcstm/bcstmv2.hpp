#pragma once

#include <3ds.h>

#include <bcstm/base.hpp>
#include <cstring>
#include <fstream>
#include <li_backend_c3d.hpp>
#include <pd.hpp>

namespace D7 {
class BCSTM2 : public BCSTMPlayerBase {
 public:
  BCSTM2() : BCSTMPlayerBase("BCSTMV2", FormatBCSTM | AllChannels) {}
  ~BCSTM2() { Stop(); }

  template <typename T>
  void Read(T& v) {
    // Check if Value could be Read
    static_assert(std::is_integral<T>::value, "Cannot Read type T");
    v = 0;  // Set value to 0 (most cases a windows problem)
    std::vector<PD::u8> buf(sizeof(T), 0);  // declare buffer
    // Read data into buffer
    pFile.read(reinterpret_cast<char*>(buf.data()), sizeof(T));
    // Loop or in be reverse loop and chift the values
    for (size_t i = 0; i < sizeof(T); i++) {
      v |= static_cast<T>(buf[pBigEndian ? sizeof(T) - 1 - i : i]) << (8 * i);
    }
  }
  template <typename T>
  void Write(const T& v) {
    // Check if Value could Write
    static_assert(std::is_integral<T>::value, "Cannot Write type T");
    std::vector<PD::u8> buf(sizeof(T), 0);  // declare buffer
    // Loop or in be reverse loop and write the values
    for (size_t i = 0; i < sizeof(T); i++) {
      buf[(pBigEndian ? sizeof(T) - 1 - i : i)] =
          buf[pBigEndian ? sizeof(T) - 1 - i : i] =
              static_cast<PD::u8>((v >> (8 * i)) & 0xFF);
    }
    // Write buffer into file
    pFile.write(reinterpret_cast<const char*>(buf.data()), sizeof(T));
  }
  template <typename T>
  T Read() {
    T o;
    Read<T>(o);
    return o;
  }

  void LoadFile(const std::string& path);
  void Stream();
  void Play();
  void Pause();
  void Stop();
  void CleanUp() { Stop(); }

  bool IsLooping() { return this->is_looping; }
  bool IsLoaded() { return this->is_loaded; }
  unsigned int GetLoopStart() { return this->loop_start; }
  unsigned int GetLoopEnd() { return this->loop_end; }
  unsigned int GetChannelCount() { return this->channel_count; }
  unsigned int GetTotal() { return this->num_blocks; }
  unsigned int GetCurrent() { return this->current_block; }
  unsigned int GetSampleRate() { return this->sample_rate; }
  unsigned int GetSamples() const {
    return (block_size * num_blocks) / sample_rate;
  }
  unsigned int GetBlcokSize() const { return block_size; }
  unsigned int GetBlockSamples() const { return block_samples; }

  bool pBigEndian = false;
  std::fstream pFile;

 private:
  unsigned int read32();
  unsigned short read16();
  unsigned char read8();
  bool file_advance(unsigned long long bytes);
  void stream();
  void fill_buffers();

  enum RefType : uint16_t {
    ByteTable = 0x0100,
    ReferenceTable = 0x0101,
    SampleData = 0x1F00,
    DSPADPCMInfo = 0x0300,
    InfoBlock = 0x4000,
    SeekBlock = 0x4001,
    DataBlock = 0x4002,
    StreamInfo = 0x4100,
    TrackInfo = 0x4101,
    ChannelInfo = 0x4102,
  };

  static const int buffer_count = 20;
  static const int max_channels = 8;

  unsigned int current_time = 0;
  unsigned int last_time = 0;

  bool is_loaded = false;
  bool is_paused = false;
  bool is_looping = false;
  bool is_streaming = false;
  bool is_little_endian;

  unsigned int channel_count = 0;
  unsigned int sample_rate = 0;

  unsigned int loop_start = 0;
  unsigned int loop_end = 0;
  unsigned int num_blocks = 0;
  unsigned int block_size = 0;
  unsigned int block_samples = 0;
  unsigned int last_block_size = 0;
  unsigned int last_block_samples = 0;
  unsigned short adpcm_coefs[max_channels][16];

  unsigned int current_block = 0;
  unsigned int info_offset = 0;
  unsigned int data_offset = 0;

  unsigned int active_channels = 0;
  unsigned short channel[max_channels];

  ndspWaveBuf wave_buf[max_channels][buffer_count];
  ndspAdpcmData adpcm_data[max_channels][2];
  PD::Vec<u8, PD::LinearAlloc<u8>> buffer_data[max_channels][buffer_count];
};
}  // namespace D7