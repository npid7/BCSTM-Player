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
  const std::string& Author() const { return pAuthor; }
  const std::string& Id() const { return pId; }
  const std::string& Name() const { return pName; }

  std::unordered_map<PD::u32, std::string> pStrings;
  std::string pAuthor;
  std::string pId;
  std::string pName;
};
}  // namespace D7