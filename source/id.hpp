#pragma once

#include <palladium>

/**
 * This stuff is still in beta but as i found out there are a lot of
 * problems with this...
 * : We can only store strings from runtime.
 *    Everything else from runtime cannot hold a name
 */

namespace D7 {
class ID {
 public:
  constexpr ID() : pID(0) {}
  constexpr ID(const PD::u32& id) : pID(id) {}
  template <size_t N>
  constexpr ID(const char (&name)[N])
      : pID(PD::FNV1A32(name)), pRaw(name), pRawLen(N - 1) {}
  ~ID() {}

  constexpr PD::u32 GetID() const { return pID; }
  constexpr operator PD::u32() const { return pID; }
  constexpr const char* GetName() const { return pRaw; }
  constexpr size_t GetNameLen() { return pRawLen; }
  constexpr bool HasName() const { return pRawLen; }

 private:
  PD::u32 pID;
  const char* pRaw = nullptr;
  size_t pRawLen = 0;
};
}  // namespace D7

namespace std {
template <>
struct hash<D7::ID> {
  constexpr std::size_t operator()(const D7::ID& id) const noexcept {
    return static_cast<std::size_t>(id.GetID());
  }
};
}  // namespace std