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
  PD::Vec<PD::Vec<ndspWaveBuf>> pWaveBuf;
  PD::Vec<PD::u8, PD::LinearAlloc<PD::u8>> pBufferData[8][20];
  // PD::Vec<PD::Vec<u8*>> pBufferData;
  // PD::Vec<PD::Vec<PD::Vec<u8, PD::LinearAlloc<u8>>> pBufferData;
};
}  // namespace D7