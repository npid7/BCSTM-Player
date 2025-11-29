#pragma once

#include <3ds.h>

#include <ctrff.hpp>
#include <li_backend_c3d.hpp>

namespace D7 {
class BcstmPlayer {
 public:
  BcstmPlayer() = default;
  ~BcstmPlayer() { CleanUp(); }

  void LoadFile(const std::string& path);
  void Stream();

  void CleanUp();

  void Play();
  void Stop();
  void Pause();

  /** INFO */
  inline bool IsLooping() { return pCurrentFile.IsLooping(); }
  inline bool IsLoaded() { return pIsLoaded; }
  inline unsigned int GetLoopStart() { return pCurrentFile.GetLoopStart(); }
  inline unsigned int GetLoopEnd() { return pCurrentFile.GetLoopEnd(); }
  inline unsigned int GetChannelCount() {
    return pCurrentFile.GetNumChannels();
  }
  inline unsigned int GetTotal() { return pCurrentFile.GetNumBlocks(); }
  inline unsigned int GetCurrent() { return pCurrentBlock; }
  inline unsigned int GetSampleRate() { return pCurrentFile.GetSampleRate(); }
  inline unsigned int GetSamples() const {
    return 0;  // (block_size * num_blocks) / sample_rate;
  }
  inline unsigned int GetBlcokSize() const {
    return pCurrentFile.GetBlockSize();
  }
  inline unsigned int GetBlockSamples() const {
    return pCurrentFile.GetBlockSamples();
  }

  void pFillBuffers();

  /** Probably unused */
  static constexpr PD::u8 MaxChannels = 8;
  static constexpr int BufferCount = 20;
  ctrff::BCSTM pCurrentFile;
  bool pIsLoaded = false;
  bool pIsStreaming = false;
  bool pIsPaused = false;

  PD::u64 pCurrentTime = 0;
  PD::u64 pLastTime = 0;

  PD::u32 pCurrentBlock = 0;
  PD::u32 pActiveChannels = 0;
  PD::Vec<PD::u16> pChannels;
  std::vector<std::vector<ndspWaveBuf>> pWaveBuf;
  std::vector<std::vector<PD::Vec<u8, PD::LinearAlloc<u8>>>> pBufferData;
};
}  // namespace D7