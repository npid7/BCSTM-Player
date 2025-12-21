#include <bcstm_ctrl.hpp>
#include <config.hpp>
#include <filebrowser.hpp>
#include <flex/flex.hpp>
#include <inspector_view.hpp>
#include <lang.hpp>
#include <msg_handler.hpp>
#include <palladium>
#include <pd-3ds.hpp>
#include <pd_ctr_ext.hpp>
#include <settings.hpp>
#include <stagemgr.hpp>
#include <stages.hpp>
#include <thread>

/**
 * Code is very unstable currently cause
 * 1. PD Ctr Extension is unfinished pre alpha
 * 2. the flex ui engine is a small lib planned to be more simple then ui7 but
 * the way it works is bad
 * 3. The whole code around the stage_mgr is just in alpha state
 * 4. The whole code of bcstm_player 2.0.0 is completly hardcoded with extern
 * references through all the code and still very unfinished
 */

D7::MsgHandler::Ref MsgHnd = nullptr;
Theme gTheme;
D7::Config pCfg;
PD::UI7::Context::Ref ui7 = nullptr;

float Offset(float x) {
  float y = cos(x) * 42;
  return y - floor(y);
}

void Append(PD::Li::DrawList::Ref l, int index, PD::fvec2 position,
            PD::fvec2 size, float time) {
  float offset = Offset(index) * 62;
  float x_position =
      position.x + size.x / 8 * ((index % 11) - 1) + cos(offset + time) * 10;
  float y_position = position.y + size.y / 8 * (index / 11) + 40 +
                     sin(offset + time) * 10 + 30;
  float color_effect = 1 - exp(-(index / 11) / 3.0f);
  l->DrawTriangleFilled(PD::fvec2(x_position, y_position),
                        PD::fvec2(x_position + 300, y_position + (90)),
                        PD::fvec2(x_position - 300, y_position + (90)),
                        PD::Color(float(.94f - .17f * color_effect),
                                  float(.61f - .25f * color_effect),
                                  float(.36f + .38f * color_effect)));
}

void BCSTM_Handler(BCSTM_Ctrl* ctrl) {
  ctrl->player = new D7::CTRFFDec;  // Stable
  while (true) {
    PD::TT::Scope st("Thread");
    if (ctrl->pFileLoaded) {
      ctrl->player->Stream();
    }
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

FileMgr::Ref Filebrowser;
Inspector::Ref FileInspector;
InspectorBCWAV::Ref FileInspectorBCWAV;
Settings::Ref Settings;
BCSTM_Ctrl bcstm_ctrl;
D7::Lang Lang;

class Progressbar : public PD::UI7::Container {
 public:
  /**
   * Constructor for the Progresbar Object
   */
  Progressbar(float v) {
    this->pVal = v;
    SetSize(PD::fvec2(310, 20));
  }
  ~Progressbar() = default;

  PD_SHARED(Progressbar);

  /**
   * Override for the Rendering Handler
   * @note This function is usally called by Menu::Update
   * */
  void Draw() override {
    auto p = FinalPos();
    list->DrawRectFilled(p, GetSize(), gTheme.ListOdd);
    list->DrawRectFilled(p + PD::fvec2(2), GetSize() - 4, gTheme.Background);

    list->DrawRectFilled(p + PD::fvec2(2),
                         PD::fvec2((GetSize().x - 4) * pVal, GetSize().y - 4),
                         gTheme.Progressbar);
  }

 private:
  float pVal;
};

void BottomScreenBeta(PD::Li::DrawList::Ref l) {
  if (ui7->BeginMenu(Lang.Get("CONTROLCENTER"),
                     UI7MenuFlags_NoCollapse | UI7MenuFlags_NoMove |
                         UI7MenuFlags_NoResize | UI7MenuFlags_Scrolling)) {
    auto m = ui7->CurrentMenu();
    float scale = 0.f;

    if (bcstm_ctrl.player->GetTotal() != 0) {
      scale = (float)bcstm_ctrl.player->GetCurrent() /
              (float)bcstm_ctrl.player->GetTotal();
    }
    m->AddObject(Progressbar::New(scale));
    if (m->Button(bcstm_ctrl.player->IsPlaying() ? "Pause" : "Play")) {
      if (bcstm_ctrl.player->IsPlaying()) {
        bcstm_ctrl.DoRequest(bcstm_ctrl.Pause);
      } else {
        bcstm_ctrl.DoRequest(bcstm_ctrl.Play);
      }
    }
    m->SameLine();
    if (m->Button("Stop")) {
      bcstm_ctrl.DoRequest(bcstm_ctrl.Stop);
    }
    m->SeparatorText("Info");
    m->Label("{}{}", Lang.Get("PLAYING"),
             std::filesystem::path(bcstm_ctrl.player->GetFilePath())
                 .filename()
                 .string());
    m->Label("Block Pos: {}/{}", bcstm_ctrl.player->GetCurrent(),
             bcstm_ctrl.player->GetTotal());
    m->Label("Loop: {}", bcstm_ctrl.player->IsLooping());
    m->Label("LoopStart/End: {}/{}", bcstm_ctrl.player->GetLoopStart(),
             bcstm_ctrl.player->GetLoopEnd());
    m->Label("Decoder: {}", bcstm_ctrl.player->GetName());
    m->Label("Stream: {}",
             PD::Strings::FormatNanos(
                 PD::OS::GetTraceRef("Stream")->GetProtocol()->GetAverage()));
    m->Label(
        "Render: {}",
        PD::Strings::FormatNanos(
            PD::OS::GetTraceRef("CtxUpdate")->GetProtocol()->GetAverage()));
    m->Label("Main: {}",
             PD::Strings::FormatNanos(
                 PD::OS::GetTraceRef("mainloop")->GetProtocol()->GetAverage()));
    ui7->EndMenu();
  }
}

std::string Clock22() {
  const time_t ut = time(0);
  bool h24 = pCfg.Get<bool>("clock_fmt24");
  bool ds = pCfg.Get<bool>("clock_seconds");
  auto ts = localtime(&ut);
  if (!h24) {
    int hr = ts->tm_hour % 12;
    if (hr == 0) hr = 12;
    if (!ds) {
      return std::format("{}:{:02} {}", hr, ts->tm_min,
                         ts->tm_hour >= 12 ? "PM" : "AM");
    }
    return std::format("{}:{:02}:{:02} {}", hr, ts->tm_min, ts->tm_sec,
                       ts->tm_hour >= 12 ? "PM" : "AM");
  }
  if (!ds) {
    return std::format("{}:{:02}", ts->tm_hour, ts->tm_min);
  }
  return std::format("{}:{:02}:{:02}", ts->tm_hour, ts->tm_min, ts->tm_sec);
}

void InitThemes() {
  std::filesystem::create_directories("sdmc:/3ds/BCSTM-Player/themes");
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
  gTheme.Load("sdmc:/3ds/BCSTM-Player/themes/" +
              pCfg.Get<std::string>("last_theme") + ".json");
}

int main() {
  PD::Ctr::EnableExceptionScreen();
  PD::Ctr::CreateContext();
  pCfg.Load("sdmc:/3ds/BCSTM-Player/config.json");
  InitThemes();
  aptSetSleepAllowed(true);
  auto ret = ndspInit();
  if (R_FAILED(ret)) {
    throw std::runtime_error(
        "ndspfirm.cdc was not found!\n\nNote: You can dump the file "
        "in\n\nRosalina Menu\n   Miscellaneous Options\n     Dump DSP "
        "Firmware\n\nThen "
        "Restart this App.");
  }
  auto font = PD::LoadSystemFont();
  // auto font = PD::Li::Font::New();
  // font->LoadTTF("romfs:/fonts/ComicNeue.ttf");
  MsgHnd = D7::MsgHandler::New(font);
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
  auto rl2 = PD::Li::DrawList::New();
  auto rl3 = PD::Li::DrawList::New();
  rl3->SetFont(font);
  Filebrowser = FileMgr::New(font);
  FileInspector = Inspector::New(font);
  FileInspectorBCWAV = InspectorBCWAV::New(font);
  Settings = Settings::New(font);
  Stage::Goto(Filebrowser);
  std::thread bcstm_player(BCSTM_Handler, &bcstm_ctrl);
  ui7 = PD::UI7::Context::New();
  ui7->GetIO()->Font = font;
  ui7->AddViewPort("VpBot", PD::ivec4(0, 0, 320, 240));
  ui7->UseViewPort("VpBot");
  PD::Timer time;
  PD::Timer time22;
  double delta;
  while (PD::Ctr::ContextUpdate()) {
    PD::TT::Scope s("mainloop");
    time.Update();
    time22.Update();
    delta = time22.GetSeconds() * 1000.0;
    time22.Reset();
    rl2->SetFont(font);

    rl2->Layer(0);
    if (pCfg.Get<bool>("rd7tfbg")) {
      rl2->DrawRectFilled(PD::fvec2(0, 0), PD::fvec2(400, 240), gTheme.HblBG0);
      for (int i = 0; i < 44; i++)
        Append(rl2, i, PD::fvec2(0, 0), PD::fvec2(400, 240),
               (float)time.GetSeconds());
    } else {
      rl2->DrawRectFilled(PD::fvec2(0, 0), PD::fvec2(400, 240),
                          gTheme.Background);
    }
    BottomScreenBeta(rl3);

    Stage::DoUpdate();
    MsgHnd->Update(delta);
    ui7->Update();
    rl2->Merge(Stage::GetDrawDataTop());
    rl2->pLayer++;
    rl2->DrawTextEx(PD::fvec2(395, 1), Clock22(), gTheme.Text,
                    LiTextFlags_AlignRight);
    rl2->pLayer--;
    rl2->Merge(MsgHnd->GetDrawList());
    rl2->Optimize();
    rl3->Merge(Stage::GetDrawDataBottom());
    rl3->Optimize();
    PD::Ctr::AddDrawList(rl2, false);
    PD::Ctr::AddDrawList(ui7->GetDrawData(), true);
    if (PD::Hid::IsDown(PD::Hid::Key::Start)) {
      break;
    }
  }
  bcstm_ctrl.DoRequest(bcstm_ctrl.KillThread);
  /** Make sure to unload before unloading the rest */
  Filebrowser = nullptr;
  FileInspector = nullptr;

  PD::Ctr::DestroyContext();
  ndspExit();
  return 0;
}
