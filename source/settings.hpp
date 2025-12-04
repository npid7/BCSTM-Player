#pragma once

#include <cursor.hpp>
#include <pd.hpp>
#include <stagemgr.hpp>
#include <thread>

class Settings : public Stage {
 public:
  Settings(PD::Hid::Ref inp, PD::LI::Texture::Ref wp, PD::LI::Font::Ref f)
      : Stage(inp, wp, f) {
    Init();
  }
  ~Settings() = default;
  PD_SMART_CTOR(Settings);

  void Update() override;
  void Init();

  Cursor cursor = Cursor(fvec2(0.f, 18.f), 17.f);
  int sp = 0;
  PD::Timer delta;
  struct TabEntry {
    TabEntry() {}
    TabEntry(const std::string& f, const std::string& s) {
      First = f;
      Second = s;
    }
    TabEntry(const std::string& f, const std::string& s,
             std::function<void(std::string&)> fu) {
      First = f;
      Second = s;
      pFunc = fu;
    }
    PD_SMART_CTOR(TabEntry);

    std::string First;
    std::string Second;
    std::function<void(std::string&)> pFunc;
    std::vector<TabEntry::Ref> SubData;
  };

  static TabEntry::Ref MakeEntry(const std::string& id, const std::string& val);

  /** THIS System of navigation is very memory inefficient btw */
  std::vector<TabEntry::Ref> pDL;
  std::vector<TabEntry::Ref> List;
  PD::Stack<std::vector<TabEntry::Ref>> pStack;
  PD::Stack<PD::Pair<int, int>> pLastPos;
};