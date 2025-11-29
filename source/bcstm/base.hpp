#pragma once

#include <string>

/**
 * Base class for In app switching between CTRFF and BCSTMV2 Decoders
 */

namespace D7 {
class BCSTMPlayerBase {
 public:
  BCSTMPlayerBase() = default;
  BCSTMPlayerBase(const std::string& n) : pName(n) {}
  virtual ~BCSTMPlayerBase() = default;

  virtual void LoadFile(const std::string& path) = 0;
  virtual void Stream() = 0;

  virtual void CleanUp() = 0;

  virtual void Play() = 0;
  virtual void Stop() = 0;
  virtual void Pause() = 0;

  /** INFO */
  virtual bool IsLooping() { return false; }
  virtual bool IsLoaded() { return false; }
  virtual unsigned int GetLoopStart() { return 0; }
  virtual unsigned int GetLoopEnd() { return 0; }
  virtual unsigned int GetChannelCount() { return 0; }
  virtual unsigned int GetTotal() { return 0; }
  virtual unsigned int GetCurrent() { return 0; }
  virtual unsigned int GetSampleRate() { return 0; }
  virtual unsigned int GetSamples() const { return 0; }
  virtual unsigned int GetBlcokSize() const { return 0; }
  virtual unsigned int GetBlockSamples() const { return 0; }

  std::string pName = "Unknown";
};
}  // namespace D7