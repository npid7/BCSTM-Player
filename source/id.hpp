#pragma once

#include <palladium>

namespace D7 {
class ID {
 public:
  constexpr ID(const PD::u32& id) { pID = id; }
  constexpr ID(std::string name) { pID = PD::FNV1A32(name); }
  constexpr ID(const char* name) { pID = PD::FNV1A32(name); }
  ~ID() {}

  constexpr PD::u32 GetID() { return pID; }
  constexpr operator PD::u32() const { return pID; }

 private:
  PD::u32 pID;
};
}  // namespace D7