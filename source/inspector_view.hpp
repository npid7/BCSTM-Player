#pragma once

#include <cursor.hpp>
#include <palladium>
#include <stagemgr.hpp>
#include <thread>

class Inspector : public Stage {
 public:
  Inspector(PD::Li::Font::Ref f) : Stage(f) {}
  ~Inspector() = default;
  PD_SHARED(Inspector)

  Cursor cursor = Cursor(PD::fvec2(0.f, 18.f), 17.f);
  int sp = 0;
  PD::Timer delta;

  void Update() override;

  void ReadFile(const std::string& path);

  struct TabEntry {
    TabEntry() {}
    PD_SHARED(TabEntry);

    std::string First;
    std::string Second;
    std::vector<TabEntry::Ref> SubData;
  };

  static TabEntry::Ref MakeEntry(const std::string& id, const std::string& val);

  /** THIS System of navigation is very memory inefficient btw */
  std::vector<TabEntry::Ref> pDL;
  std::vector<TabEntry::Ref> List;
  std::stack<std::vector<TabEntry::Ref>> pStack;
  std::stack<std::pair<int, int>> pLastPos;
  std::string pPath;
};