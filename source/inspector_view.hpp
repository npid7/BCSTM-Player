#pragma once

#include <cursor.hpp>
#include <pd.hpp>
#include <stagemgr.hpp>
#include <thread>

class Inspector : public Stage {
 public:
  Inspector(PD::Hid::Ref inp, PD::LI::Texture::Ref wp, PD::LI::Font::Ref f)
      : Stage(inp, wp, f) {}
  ~Inspector() = default;
  PD_SMART_CTOR(Inspector)

  Cursor cursor = Cursor(fvec2(0.f, 18.f), 17.f);
  int sp = 0;
  PD::Timer delta;

  void Update() override;

  void ReadFile(const std::string& path);

  struct TabEntry {
    TabEntry() {}
    PD_SMART_CTOR(TabEntry);

    std::string First;
    std::string Second;
    std::vector<TabEntry::Ref> SubData;
  };

  static TabEntry::Ref MakeEntry(const std::string& id, const std::string& val);

  /** THIS System of navigation is very memory inefficient btw */
  std::vector<TabEntry::Ref> pDL;
  std::vector<TabEntry::Ref> List;
  PD::Stack<std::vector<TabEntry::Ref>> pStack;
  PD::Stack<PD::Pair<int, int>> pLastPos;
  std::string pPath;
};