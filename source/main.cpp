#include <app.hpp>
#include <bcstm_ctrl.hpp>
#include <config.hpp>
#include <filebrowser.hpp>
#include <info.hpp>
#include <iostream>
#include <lang.hpp>
#include <msg_handler.hpp>
#include <palladium>
#include <pd_ctr_ext.hpp>
#include <progressbar.hpp>
#include <stagemgr.hpp>
#include <thread>

class BCSTMPlayer : public D7::App {
 public:
  BCSTMPlayer() {
    PD::Ctr::CreateContext();
    PD::Ctr::AddRenderCallbackTop([=, this]() { RenderTop(); });
    PD::Ctr::AddRenderCallbackBottom([=, this]() { RenderBot(); });
    auto ret = ndspInit();
    std::filesystem::create_directories("sdmc:/3ds/BCSTM-Player/themes");
    if (R_FAILED(ret)) {
      throw std::runtime_error(
          "ndspfirm.cdc was not found!\n\nNote: You can dump the file "
          "in\n\nRosalina Menu\n   Miscellaneous Options\n     Dump DSP "
          "Firmware\n\nThen "
          "Restart this App.");
    }
    pCfg.Load("sdmc:/3ds/BCSTM-Player/config.json");
    InitThemes();
    Font.LoadTTF("romfs:/fonts/ComicNeue.ttf", 32, LiFontFlags_SDF);
    MsgHnd = D7::MsgHandler::New(&Font);
    pTop.SetFont(&Font);
    // 24 * 0.7 -> 16.8
    // 16.8 / 32 = 0.525
    pTop.SetFontscale(0.525);
    ui7.GetIO().Font = &Font;
    ui7.GetIO().WrapLabels = true;  // Beta
    ui7.GetIO().FontScale = 0.525;
    ui7.GetIO().ShowMenuBorder = false;
    ui7.AddViewPort("VpBot", PD::ivec4(0, 0, 320, 240));
    pFileMgr = FileMgr::New(&Font, Lang, ctrl);
    pFileMgr->ScanDir("sdmc:/");
    Stage::SetTheme(pTheme);
    Stage::Goto(pFileMgr);
    LoadLanguage();
    bcstm_loop = std::thread(&BCSTMPlayer::BcstmLoop, this, &ctrl);
  }
  ~BCSTMPlayer() {
    ctrl.DoRequest(ctrl.KillThread);
    MsgHnd.reset();
    ndspExit();
    PD::Ctr::DestroyContext();
  }

  void Main() override {
    PD::TT::Beg("Main");
    if (!pShowLicense && pLicenseText.size()) {
      pLicenseText.clear();
    }
    if (!pShowSettings && pCfg.Updated()) {
      pCfg.Save("sdmc:/3ds/BCSTM-Player/config.json");
    }
    PD::Hid::Update();
    if (!pNoUI7) {
      ui7.UseViewPort("VpBot");
      bool lc = pShowLicense;
      bool* close_ptr = nullptr;
      std::string title = Lang.Get("CONTROLCENTER");
      if (pShowLicense) {
        close_ptr = &pShowLicense;
        title = Lang.Get("LICENSE");
      } else if (pShowSettings) {
        close_ptr = &pShowSettings;
        title = Lang.Get("SETTINGS");
      }
      if (auto m = ui7.BeginMenu(title, pUI7Flags, close_ptr)) {
        m->SetSize(PD::fvec2(320, 240));
        m->SetPosition(0);
        if (pShowLicense) {
          LicenseWindow(m);
        } else if (!lc && pShowSettings) {
          SettingsWindow(m);
        } else {
          MainWindow(m);
        }
        ui7.EndMenu();
      }
      ui7.StyleEditor(&pShowStyleEditor);
      ui7.MetricsMenu(&pShowMetrics);
      ui7.Update();
    }
    // Top Screen
    DrawBG();
    Stage::DoUpdate();
    pTop.Merge(Stage::GetDrawDataTop());
    pTop.Optimize();
    DrawClock();
    MsgHnd->Update(Delta());
    PD::TT::End("Main");
    if (pAllowNoUI7) {
      pNoUI7 = PD::Hid::IsEvent(PD::Hid::Event::Held, PD::Hid::Gamepad::L);
    }
    if (!PD::Ctr::ContextUpdate() ||
        PD::Hid::IsEvent(PD::Hid::Event::Up, PD::Hid::Gamepad::Start)) {
      if (pCfg.Updated()) {
        pCfg.Save("sdmc:/3ds/BCSTM-Player/config.json");
      }
      Exit();
    }
  }

  void RenderTop() {
    PD::Gfx::SetViewPort(PD::fvec2(400, 240));
    PD::Gfx::Draw(pTop);
    PD::Gfx::Draw(MsgHnd->GetDrawList());
    pTop.Clear();
  }

  void RenderBot() {
    PD::Gfx::SetViewPort(PD::fvec2(320, 240));
    PD::Gfx::Draw(ui7.GetDrawData());
  }

  void LoadLanguage() {
    if (pCfg.Get<std::string>("last_lang") == "sys") {
      try {
        Lang.Load("romfs:/lang/" + PD::Ctr::GetSystemLanguage() + ".json");
      } catch (const std::runtime_error& e) {
        MsgHnd->Push("Lang Error:", e.what());
        Lang.Load("romfs:/lang/en.json");
      }
    } else {
      Lang.Load("romfs:/lang/" + pCfg.Get<std::string>("last_lang") + ".json");
    }
    pLanguages.clear();
    pLanguages.push_back("sys");
    for (auto& it : std::filesystem::directory_iterator("romfs:/lang/")) {
      pLanguages.push_back(it.path().filename().stem().string());
    }
  }

  void SettingsWindow(PD::UI7::Menu* m) {
    m->SeparatorText("UI");
    m->Checkbox(Lang.Get("RD7TFBG"), pCfg.GetBool("rd7tfbg"));
    if (m->Button("Style Editor")) {
      pShowStyleEditor = true;
      ui7.FocusMenu("UI7 Style Editor");
    }
    if (m->Button("Metrics")) {
      pShowMetrics = true;
      ui7.FocusMenu("UI7 Metrics");
    }
    m->SeparatorText(Lang.Get("CLOCK"));
    m->Checkbox(Lang.Get("24HRS"), pCfg.GetBool("clock_fmt24"));
    m->Checkbox(Lang.Get("SHOWSECONDS"), pCfg.GetBool("clock_seconds"));
    m->SeparatorText(Lang.Get("DECODER"));
    m->Label("{}: {}", Lang.Get("Decoder"), ctrl.player->GetName());
    m->Label("{}: {}", Lang.Get("CHANNEL_SUPPORT"),
             ctrl.player->FeatureChannels(ctrl.player->pFeatures));
    m->Label("{}: {}", Lang.Get("FILE_SUPPORT"),
             ctrl.player->FeatureFiles(ctrl.player->pFeatures));
    m->Label("{}: {}", Lang.Get("ENCODING_SUPPORT"),
             ctrl.player->FeatureEncoding(ctrl.player->pFeatures));
    if (m->Button(ctrl.player->GetName() == "CTRFFDec" ? "BCSTMV2 (Legacy)"
                                                       : "CTRFFDec")) {
      ctrl.DoRequest(ctrl.SwitchDec);
    }
    if (m->BeginTreeNode(Lang.Get("LANGUAGE"))) {
      for (auto& it : pLanguages) {
        if (m->Button(it)) {
          pCfg.Set("last_lang", it);
          LoadLanguage();
        }
      }

      m->EndTreeNode();
    }
    if (m->BeginTreeNode(Lang.Get("CREDITS"))) {
      m->Label("tobid7");
      m->Label(Lang.Get("CREDIT_TOBID7"));
      m->Separator();
      m->Label("devkitpro");
      m->Label(Lang.Get("CREDIT_DEVKITPRO"));
      m->Separator();
      m->Label("cheuble");
      m->Label(Lang.Get("CREDIT_CHEUBLE"));
      m->Separator();
      m->Label("3DBrew");
      m->Label(Lang.Get("CREDIT_3DBREW"));
      m->Separator();
      m->Label("crozynski");
      m->Label(Lang.Get("CREDIT_CROZYNSKI"));
      if (m->Button("Show License")) {
        pLicenseText = pParseLicense("romfs:/license/ComicNeue.txt");
        pShowLicense = true;
      }
      m->EndTreeNode();
    }
    if (m->BeginTreeNode(Lang.Get("SHOW_INFO"))) {
      m->Label("Version: {}", VERSION);
      m->Label("Commit: {}", GIT_COMMIT);
      m->Label("Branch: {}", GIT_BRANCH);
      m->Label("C++: {} ({}-{})", (__cplusplus / 100) % 100, __cplusplus / 100,
               __cplusplus % 100);
      m->Label("Compiler: {}", PD::Strings::GetCompilerVersion());
      m->Label("RTTI: {}", D7::Cxx::RTTIEnabled());
      m->Label("Exceptions: {}", D7::Cxx::ExceptionsEnabled());
      m->EndTreeNode();
    }
    if (m->BeginTreeNode(Lang.Get("DEVELOPER"))) {
      m->Checkbox(Lang.Get("SHOWDEBUG"), pDebugDelta);
      m->Checkbox(Lang.Get("ALLOWNOUI7"), pAllowNoUI7);
      m->EndTreeNode();
    }
  }

  void LicenseWindow(PD::UI7::Menu* m) { m->Label(pLicenseText); }

  void MainWindow(PD::UI7::Menu* m) {
    if (m->Button(Lang.Get("SETTINGS"))) {
      pShowSettings = true;
    }
    float scale = 0.f;

    if (ctrl.player->GetTotal() != 0) {
      scale = (float)ctrl.player->GetCurrent() / (float)ctrl.player->GetTotal();
    }
    // m->AddObject(Progressbar::New(scale));
    if (m->Button(ctrl.player->IsPlaying() ? "Pause" : "Play")) {
      if (ctrl.player->IsPlaying()) {
        ctrl.DoRequest(ctrl.Pause);
      } else {
        ctrl.DoRequest(ctrl.Play);
      }
    }
    m->SameLine();
    if (m->Button("Stop")) {
      ctrl.DoRequest(ctrl.Stop);
    }
    m->SeparatorText("Info");
    m->Label(
        "{}{}", Lang.Get("PLAYING"),
        std::filesystem::path(ctrl.player->GetFilePath()).filename().string());
    m->Label("Block Pos: {}/{}", ctrl.player->GetCurrent(),
             ctrl.player->GetTotal());
    m->Label("Loop: {}", ctrl.player->IsLooping());
    m->Label("LoopStart/End: {}/{}", ctrl.player->GetLoopStart(),
             ctrl.player->GetLoopEnd());
    m->Label("Decoder: {}", ctrl.player->GetName());
    m->Label("Stream: {}",
             PD::Strings::FormatNanos(
                 PD::TT::GetTraceRef("Thread").GetProtocol().GetAverage()));
    m->Label("Render: {}",
             PD::Strings::FormatNanos(
                 PD::TT::GetTraceRef("CtxUpdate").GetProtocol().GetAverage()));
    m->Label("Main: {}",
             PD::Strings::FormatNanos(
                 PD::TT::GetTraceRef("Main").GetProtocol().GetAverage()));
  }

  void DrawClock() {
    std::string str;
    const time_t ut = time(0);
    bool h24 = pCfg.Get<bool>("clock_fmt24");
    bool ds = pCfg.Get<bool>("clock_seconds");
    auto ts = localtime(&ut);
    if (h24) {
      if (ds) {
        str =
            std::format("{}:{:02}:{:02}", ts->tm_hour, ts->tm_min, ts->tm_sec);

      } else {
        str = std::format("{}:{:02}", ts->tm_hour, ts->tm_min);
      }
    } else {
      int hr = ts->tm_hour % 12;
      if (hr == 0) hr = 12;
      if (ds) {
        str = std::format("{}:{:02}:{:02} {}", hr, ts->tm_min, ts->tm_sec,
                          ts->tm_hour >= 12 ? "PM" : "AM");
      } else {
        str = std::format("{}:{:02} {}", hr, ts->tm_min,
                          ts->tm_hour >= 12 ? "PM" : "AM");
      }
    }
    std::string str2;
    if (pDebugDelta) {
      str2 = std::format(
          "{} ", PD::Strings::FormatNanos(
                     PD::TT::GetTraceRef("Main").GetProtocol().GetAverage()));
      DebugOverlay();
    }
    pTop.DrawTextEx(PD::fvec2(395, 1), std::string(str2 + str).c_str(),
                    pTheme.Text, LiTextFlags_AlignRight);
  }

  void DrawBG() {
    // pTop.Layer(0);
    if (pCfg.Get<bool>("rd7tfbg")) {
      pTop.DrawRectFilled(PD::fvec2(0, 0), PD::fvec2(400, 240), pTheme.HblBG0);
      for (int i = 0; i < 44; i++)
        pAppend(pTop, i, PD::fvec2(0, 0), PD::fvec2(400, 240), Time());
    } else {
      pTop.DrawRectFilled(PD::fvec2(0, 0), PD::fvec2(400, 240),
                          pTheme.Background);
    }
  }

  void BcstmLoop(BCSTM_Ctrl* ctrl) {
    ctrl->player = new D7::CTRFFDec;  // Stable
    while (true) {
      PD::TT::Scope st("Thread");
      ctrl->player->Stream();
      if (ctrl->pRequests.size() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
      } else {
        auto t = ctrl->pRequests.front();
        if (t.req == BCSTM_Ctrl::OpenFile) {
#if __EXCEPTIONS
          try {
            ctrl->player->LoadFile(t.req_dat);
            /**
             * Set to true cause it would get set to false
             * if an error got thrown
             */
            ctrl->pFileLoaded = true;
          } catch (const std::exception& e) {
            MsgHnd->Push(ctrl->player->GetName(), e.what());
            std::cout << "BCSTM CTRL Error: " << e.what() << std::endl;
            ctrl->pFileLoaded = false;
          }
#else
          ctrl->player->LoadFile(t.req_dat);
          ctrl->pFileLoaded = true;
#endif
        } else if (t.req == BCSTM_Ctrl::CloseFile) {
          ctrl->player->Stop();
        } else if (t.req == BCSTM_Ctrl::Stop) {
          ctrl->player->Stop();
        } else if (t.req == BCSTM_Ctrl::Play) {
          ctrl->player->Play();
        } else if (t.req == BCSTM_Ctrl::Pause) {
          ctrl->player->Pause();
        } else if (t.req == BCSTM_Ctrl::KillThread) {
          break; /** Break the loop */
        } else if (t.req == BCSTM_Ctrl::SwitchDec) {
          ctrl->player->Stop();
          bool v2 = ctrl->player->GetName() == "BCSTMV2 (Lagacy)";
          delete ctrl->player;
          if (v2) {
            ctrl->player = new D7::CTRFFDec();
          } else {
            ctrl->player = new D7::BCSTM2();
          }
        }
        ctrl->pRequests.pop_front();
      }
    }
  }

  void InitThemes() {
    u8* d = new u8[1024];
    for (auto& it : std::filesystem::directory_iterator("romfs:/themes/")) {
      auto p = "sdmc:/3ds/BCSTM-Player/themes/" + it.path().filename().string();
      if (std::filesystem::exists(p)) {
        continue;
      }
      std::ifstream iff(it.path().string(), std::ios::binary);
      std::ofstream off(p, std::ios::binary);

      while (true) {
        iff.read((char*)d, 1024);
        size_t rb = iff.gcount();
        if (rb) {
          off.write((const char*)d, rb);
        }
        if (rb != 1024) {
          break;
        }
      }
      iff.close();
      off.close();
    }
    delete[] d;
    pTheme.Load("sdmc:/3ds/BCSTM-Player/themes/" +
                pCfg.Get<std::string>("last_theme") + ".json");
  }

 private:
  float pOffset(float x) {
    float y = cos(x) * 42;
    return y - floor(y);
  }

  void pAppend(PD::Li::Drawlist& l, int index, PD::fvec2 position,
               PD::fvec2 size, float time) {
    float offset = pOffset(index) * 62;
    float x_position =
        position.x + size.x / 8 * ((index % 11) - 1) + cos(offset + time) * 10;
    float y_position = position.y + size.y / 8 * (index / 11) + 40 +
                       sin(offset + time) * 10 + 30;
    float color_effect = 1 - exp(-(index / 11) / 3.0f);
    l.DrawTriangleFilled(PD::fvec2(x_position, y_position),
                         PD::fvec2(x_position + 300, y_position + (90)),
                         PD::fvec2(x_position - 300, y_position + (90)),
                         PD::Color(float(.94f - .17f * color_effect),
                                   float(.61f - .25f * color_effect),
                                   float(.36f + .38f * color_effect)));
  }

  std::string pParseLicense(const std::string& path) {
    std::string ret;
    std::ifstream iff(path);
    if (iff) {
      std::string tmp;
      while (std::getline(iff, tmp)) {
        ret += tmp + "\n";
      }
      iff.close();
    }
    return ret;
  }

  void DebugOverlay() {}

  bool pShowStyleEditor = false;
  bool pShowMetrics = false;
  D7::Config pCfg;
  Theme pTheme;
  D7::Lang Lang;
  BCSTM_Ctrl ctrl;
  std::thread bcstm_loop;
  FileMgr::Ref pFileMgr = nullptr;
  PD::Li::Font Font;
  PD::Li::Drawlist pTop;
  D7::MsgHandler::Ref MsgHnd = nullptr;
  PD::UI7::Context ui7;
  bool pShowSettings = false;
  bool pShowLicense = false;
  bool pDebugDelta = false;
  bool pNoUI7 = false;
  bool pAllowNoUI7 = false;
  std::vector<std::string> pLanguages;
  std::string pLicenseText;
  const UI7MenuFlags pUI7Flags = UI7MenuFlags_NoCollapse | UI7MenuFlags_NoMove |
                                 UI7MenuFlags_NoResize |
                                 UI7MenuFlags_VtScrolling;
};

int main() {
  PD::Ctr::EnableExceptionScreen();
  BCSTMPlayer app;
  app.Run();
  return 0;
}