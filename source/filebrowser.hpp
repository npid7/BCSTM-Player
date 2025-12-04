#pragma once

#include <bcstm/bcstmv2.hpp>
#include <cursor.hpp>
#include <pd.hpp>
#include <stagemgr.hpp>
#include <thread>

class FileMgr : public Stage {
 public:
  FileMgr(PD::Hid::Ref inp, PD::LI::Texture::Ref wp, PD::LI::Font::Ref f)
      : Stage(inp, wp, f) {
    ScanDir("sdmc:/");
  }
  ~FileMgr() = default;
  PD_SMART_CTOR(FileMgr)

  Cursor cursor = Cursor(fvec2(0.f, 18.f), 17.f);
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