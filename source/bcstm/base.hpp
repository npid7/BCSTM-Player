#pragma once

#include <string>

/**
 * Base class for In app switching between CTRFF and BCSTMV2 Decoders
 */

namespace D7 {
class BCSTMPlayerBase {
 public:
  using Features = unsigned int;
  enum Features_ : Features {
    None = 0,
    SingleChannel = 1 << 0,
    DualChannel = 1 << 1,
    QuadraChannel = 1 << 2,
    HexaChannel = 1 << 3,
    OktaChannel = 1 << 4,
    AllChannels =
        SingleChannel | DualChannel | QuadraChannel | HexaChannel | OktaChannel,
    NonLoopEnd = 1 << 5,
    FormatBCSTM = 1 << 6,
    EncodingPCM8 = 1 << 7,
    EncodingPCM16 = 1 << 8,
    EncodingADPCM = 1 << 9,
    AllEncodings = EncodingADPCM | EncodingPCM16 | EncodingPCM8,
    Default = FormatBCSTM | AllChannels | EncodingADPCM,  // supported by all
  };
  // clang-format off
  static std::string FeatureChannels(Features f) {
    std::string ret;
    if (f & SingleChannel) ret += "1 ";
    if (f & DualChannel)   ret += "2 ";
    if (f & QuadraChannel) ret += "4 ";
    if (f & HexaChannel)   ret += "6 ";
    if (f & OktaChannel)   ret += "8";
    return ret;
  }

  static std::string FeatureFiles(Features f) {
    std::string ret;
    if (f & FormatBCSTM) ret += "BCSTM";
    return ret;
  }

  static std::string FeatureEncoding(Features f) {
    std::string ret;
    if(f & EncodingADPCM) ret += "ADPCM ";
    if(f & EncodingPCM8) ret += "PCM8 ";
    if(f & EncodingPCM16) ret += "PCM16 ";
    return ret;
  }
  // clang-format on
  BCSTMPlayerBase() = default;
  BCSTMPlayerBase(const std::string& n, Features f) : pName(n), pFeatures(f) {}
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
  virtual bool IsPlaying() { return false; }
  virtual unsigned int GetLoopStart() { return 0; }
  virtual unsigned int GetLoopEnd() { return 0; }
  virtual unsigned int GetChannelCount() { return 0; }
  virtual unsigned int GetTotal() { return 0; }
  virtual unsigned int GetCurrent() { return 0; }
  virtual unsigned int GetSampleRate() { return 0; }
  virtual unsigned int GetSamples() const { return 0; }
  virtual unsigned int GetBlcokSize() const { return 0; }
  virtual unsigned int GetBlockSamples() const { return 0; }
  virtual unsigned int ActiveChannels() const { return 0; }
  virtual const std::string GetFilePath() const { return ""; }

  std::string GetName() const { return pName; }

  std::string pName = "Unknown";
  Features pFeatures = Default;
};
}  // namespace D7