#pragma once

#include <bcstm/bcstmv2.hpp>
#include <cursor.hpp>
#include <palladium>
#include <stagemgr.hpp>
#include <thread>

class FileMgr : public Stage {
 public:
  FileMgr(PD::Li::Font::Ref f) : Stage(f) {
    ScanDir("sdmc:/");
    PD::Image::Ref img = PD::Image::New("romfs:/gfx/folder_small.png");
    pIcnFolder =
        PD::Gfx::LoadTex(img->pBuffer, img->Width(), img->Height(),
                         PD::Li::Texture::RGBA32, PD::Li::Texture::NEAREST);
    img = PD::Image::New("romfs:/gfx/file_small.png");
    pIcnFile =
        PD::Gfx::LoadTex(img->pBuffer, img->Width(), img->Height(),
                         PD::Li::Texture::RGBA32, PD::Li::Texture::NEAREST);
    img = PD::Image::New("romfs:/gfx/audiofile_small.png");
    pIcnAudio =
        PD::Gfx::LoadTex(img->pBuffer, img->Width(), img->Height(),
                         PD::Li::Texture::RGBA32, PD::Li::Texture::NEAREST);
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
  PD::Li::Texture::Ref pIcnFolder = nullptr;
  PD::Li::Texture::Ref pIcnFile = nullptr;
  PD::Li::Texture::Ref pIcnAudio = nullptr;
  bool pShowHelp = false;

  static void SortList(std::vector<FSEntry>& l);
  static void BackgroundListUpdate(std::vector<FSEntry>& l,
                                   const std::string& path);
  void ScanDir(const std::string& path);
};