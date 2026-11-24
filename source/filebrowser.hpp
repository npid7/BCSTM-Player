#pragma once

#include <bcstm/bcstmv2.hpp>
#include <bcstm_ctrl.hpp>
#include <cursor.hpp>
#include <lang.hpp>
#include <palladium>
#include <stagemgr.hpp>
#include <thread>

class FileMgr : public Stage {
 public:
  FileMgr(PD::Li::Font* f, D7::Lang& l, BCSTM_Ctrl& c)
      : Stage(f), Lang(l), ctrl(c) {
    ScanDir("sdmc:/");
    PD::Image img;
    img.Load("romfs:/gfx/folder_small.png");
    pIcnFolder = PD::Gfx::LoadTexture(img.data(), img.Width(), img.Height(),
                                      PD::TextureFormat::RGBA32,
                                      PD::TextureFilter::Nearest);
    img.Load("romfs:/gfx/file_small.png");
    pIcnFile = PD::Gfx::LoadTexture(img.data(), img.Width(), img.Height(),
                                    PD::TextureFormat::RGBA32,
                                    PD::TextureFilter::Nearest);
    img.Load("romfs:/gfx/audiofile_small.png");
    pIcnAudio = PD::Gfx::LoadTexture(img.data(), img.Width(), img.Height(),
                                     PD::TextureFormat::RGBA32,
                                     PD::TextureFilter::Nearest);
  }
  ~FileMgr() = default;
  PD_SHARED(FileMgr)

  Cursor cursor = Cursor(PD::fvec2(0.f, 18.f), 17.f);
  std::stack<std::pair<int, int>> pLastPos;
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
  PD::Li::Texture pIcnFolder;
  PD::Li::Texture pIcnFile;
  PD::Li::Texture pIcnAudio;
  D7::Lang& Lang;
  BCSTM_Ctrl& ctrl;
  bool pShowHelp = false;

  static void SortList(std::vector<FSEntry>& l);
  static void BackgroundListUpdate(std::vector<FSEntry>& l,
                                   const std::string& path);
  void ScanDir(const std::string& path);
};