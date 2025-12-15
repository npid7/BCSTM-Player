#pragma once

#include <id.hpp>

namespace D7 {
class Lang {
 public:
  Lang() = default;
  Lang(const std::string& path) { Load(path); }
  ~Lang() = default;

  void Load(const std::string& path);
  const std::string& Get(const ID& id);

  std::unordered_map<PD::u32, std::string> pStrings;
};
}  // namespace D7