#pragma once

#include <bcstm/bcstmv2.hpp>
#include <cursor.hpp>
#include <palladium>
#include <stagemgr.hpp>
#include <thread>

class FileMgr : public Stage {
 public:
  FileMgr(PD::Li::Font::Ref f) : Stage(f) { ScanDir("sdmc:/"); }
  ~FileMgr() = default;
  PD_SHARED(FileMgr)

  Cursor cursor = Cursor(PD::fvec2(0.f, 18.f), 17.f);
  PD::Stack<PD::Pair<int, int>> pLastPos;
  int sp = 0;
  PD::Timer delta;

  void Update() override;

  struct FSEntry {
    bool Dir;
    std::string Path;
    std::string Name;
  };
  std::vector<FSEntry> list;
  std::string cPath;
  bool pShowHelp = false;

  static void SortList(std::vector<FSEntry>& l);
  static void BackgroundListUpdate(std::vector<FSEntry>& l,
                                   const std::string& path);
  void ScanDir(const std::string& path);
};