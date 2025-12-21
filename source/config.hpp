#pragma once

#include <id.hpp>
#include <pd/external/json.hpp>

/**
 * If you read this quit the file please
 */

namespace D7 {
class Config {
 public:
  Config() = default;
  Config(const std::string& path) { Load(path); }
  ~Config() = default;

  void Load(const std::string& path) {
    std::fstream iff(path, std::ios::in);
    if (!iff.is_open()) {
      Default(path);
      return;
    }
    nlohmann::json js;
    iff >> js;
    iff.close();
    // Need to call DefaultVars to havw the names cached with the IDS
    Dvars();
    for (auto& it : js.items()) {
      PD::u32 h = PD::FNV1A32(it.key());
      pData[ID(h)] = it.value();
    }
    if (Get<int>("version") == 0) {
    }
  }

  template <typename T>
  T Get(const ID& id) {
    return pData[id].get<T>();
  }

  // Explicitly use std::string for const char
  void Set(const ID& id, const char* t) {
    pData[id] = std::string(t);
    pUpdated = true;
  }

  template <typename T>
  void Set(const ID& id, T&& t) {
    static_assert(!std::is_pointer_v<std::decay_t<T>>,
                  "[D7] Config: Pointers wil lead to invalid memory access");

    pData[id] = nlohmann::json(std::forward<T>(t));
    pUpdated = true;
  }

  void Save(const std::string& path) {
    /**
     * Best way i found to carry this func is loading the generated json
     * and edit the variables that got a Name by Settings
     * THIS WILL STILL NOT FIX THE PROBLEM OF WHAT WOULD HAPPEN IF WE DO RUNTIME
     * STUFF
     */
    pUpdated = false;
    nlohmann::json js;
    std::ifstream iff(path);
    if (iff) {
      iff >> js;
    }
    iff.close();
    for (auto& it : pData) {
      if (it.first.HasName()) {
        js[it.first.GetName()] = it.second;
      }
    }
    try {
      js.dump(4);
    } catch (const nlohmann::json::exception& e) {
      throw std::runtime_error(e.what());
    }
    std::ofstream off(path);
    off << js.dump(4);
    off.close();
  }

  void Default(const std::string& path) {
    // Set defaults
    Dvars();
    Save(path);
    Load(path);
  }

  void Dvars() {
    Set("version", 0);
    Set("clock_fmt24", true);
    Set("clock_seconds", true);
    Set("last_theme", "default");
    Set("last_lang", "sys");
    Set("rd7tfbg", true);
  }

  bool Updated() const { return pUpdated; }

  std::unordered_map<D7::ID, nlohmann::json> pData;
  bool pUpdated = true;
};
}  // namespace D7