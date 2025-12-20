#pragma once

#include <3ds.h>

#include <bcstm/base.hpp>
#include <ctrff.hpp>
#include <pd-3ds/bknd-gfx.hpp>

namespace D7 {
class CTRFFDec : public BCSTMPlayerBase {
 public:
  CTRFFDec()
      : BCSTMPlayerBase("CTRFFDec", FormatBCSTM | AllChannels | AllEncodings) {
    CleanUp();
  };
  ~CTRFFDec() { CleanUp(); }

  void LoadFile(const std::string &path);
  void Stream();

  void CleanUp();

  void Play();
  void Stop();
  void Pause();

  /** INFO */
  inline bool IsLooping() { return pCurrentFile.IsLooping(); }
  inline bool IsLoaded() { return pIsLoaded; }
  inline bool IsPlaying() { return !pIsPaused; }
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
  inline unsigned int ActiveChannels() const { return pActiveChannels; }

  void pFillBuffers();

  static constexpr int BufferCount = 20;
  ctrff::BCSTM pCurrentFile;
  bool pIsLoaded = false;
  bool pIsStreaming = false;
  bool pIsPaused = false;
  bool pIsEnding = false;

  PD::u64 pCurrentTime = 0;
  PD::u64 pLastTime = 0;

  PD::u32 pCurrentBlock = 0;
  PD::u32 pActiveChannels = 0;
  std::vector<PD::u16> pChannels;
  std::vector<std::vector<ndspWaveBuf>> pWaveBuf;
  std::vector<std::vector<std::vector<u8, PD::LinearAllocator<u8>>>>
      pBufferData;
};
}  // namespace D7