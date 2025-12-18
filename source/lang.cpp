#include <lang.hpp>

std::string pLangErr = "[KEY_NOT_FOUND]";

std::map<PD::u32, std::string> pDupeCheck;

void D7::Lang::Load(const std::string& path) {
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("[Lang]: language file " + path +
                             " doesn't exist!");
  }
  nlohmann::json js;
  std::ifstream iff(path, std::ios::in);
  iff >> js;
  iff.close();
  nlohmann::json js2 = js["keys"];
  for (auto& it : js2.items()) {
    auto id = ID(it.key());
    auto e = pDupeCheck.find(id);
    if (e != pDupeCheck.end()) {
      if (it.key() != e->second) {
        throw std::runtime_error("[Lang]: " + e->second + " and " + it.key() +
                                 " have the same hash!");
      }
    }
    pStrings[ID(it.key())] = it.value().get<std::string>();
  }
  pDupeCheck.clear();
  pName = js["info"]["name"].get<std::string>();
  pAuthor = js["info"]["author"].get<std::string>();
  pId = js["info"]["shortcut"].get<std::string>();
}

const std::string& D7::Lang::Get(const ID& id) {
  auto s = pStrings.find(id);
  if (s != pStrings.end()) {
    return s->second;
  }
  return pLangErr;
}