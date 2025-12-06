#include <bcstm_ctrl.hpp>
#include <filebrowser.hpp>
#include <flex/flex.hpp>
#include <inspector_view.hpp>
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
    if (ctrl->pFileLoaded) {
      ctrl->player->Stream();
    }
    if (ctrl->pRequests.Size() == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } else {
      auto t = ctrl->pRequests.Front();
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
      ctrl->pRequests.PopFront();
    }
  }
}

FileMgr::Ref Filebrowser;
Inspector::Ref FileInspector;
InspectorBCWAV::Ref FileInspectorBCWAV;
Settings::Ref Settings;
BCSTM_Ctrl bcstm_ctrl;

void BottomScreenBeta(PD::Li::DrawList::Ref l) {
  l->DrawRectFilled(0, PD::fvec2(320, 240), 0xff222222);
  l->DrawRectFilled(5, PD::fvec2(310, 20), 0xff111111);
  l->DrawRectFilled(7, PD::fvec2(306, 16), 0xff222222);
  float scale = 0.f;

  if (bcstm_ctrl.player->GetTotal() != 0) {
    scale = (float)bcstm_ctrl.player->GetCurrent() /
            (float)bcstm_ctrl.player->GetTotal();
  }
  l->DrawRectFilled(7, PD::fvec2(306 * scale, 16), 0xff00ff00);
  l->DrawText(
      PD::fvec2(7, 28),
      std::format(
          "Info:\n  Block Pos: {}/{}\n  Sample Rate: {}\n  Loop: {}\n  "
          "Loop Start: "
          "{}\n  Loop End: {}\n  Decoder: {}",
          bcstm_ctrl.player->GetCurrent(), bcstm_ctrl.player->GetTotal(),
          bcstm_ctrl.player->GetSampleRate(), bcstm_ctrl.player->IsLooping(),
          bcstm_ctrl.player->GetLoopStart(), bcstm_ctrl.player->GetLoopEnd(),
          bcstm_ctrl.player->GetName()),
      0xffffffff);
}

int main() {
  PD::Ctr::EnableExceptionScreen();
  PD::Ctr::CreateContext();
  aptSetSleepAllowed(true);
  auto ret = ndspInit();
  if (R_FAILED(ret)) {
    throw std::runtime_error(
        "ndspfirm.cdc was not found!\n\nNote: You can dump the file "
        "in\n\nRosalina Menu\n   Miscellaneous Options\n     Dump DSP "
        "Firmware\n\nThen "
        "Restart this App.");
  }
  std::thread bcstm_player(BCSTM_Handler, &bcstm_ctrl);
  auto font = PD::Li::Font::New();
  font->LoadTTF("romfs:/fonts/ComicNeue.ttf");
  auto rl2 = PD::Li::DrawList::New();
  auto rl3 = PD::Li::DrawList::New();
  rl3->SetFont(font);
  Filebrowser = FileMgr::New(font);
  FileInspector = Inspector::New(font);
  FileInspectorBCWAV = InspectorBCWAV::New(font);
  Settings = Settings::New(font);
  Stage::Goto(Filebrowser);

  PD::Timer time;
  while (PD::Ctr::ContextUpdate()) {
    PD::TT::Scope s("mainloop");
    time.Update();
    rl2->SetFont(font);

    rl2->DrawRectFilled(PD::fvec2(0, 0), PD::fvec2(400, 240), 0xff64c9fd);
    for (int i = 0; i < 44; i++)
      Append(rl2, i, PD::fvec2(0, 0), PD::fvec2(400, 240),
             (float)time.GetSeconds());
    BottomScreenBeta(rl3);

    Stage::DoUpdate();
    PD::Ctr::AddDrawList(rl2, false);
    PD::Ctr::AddDrawList(rl3, true);
    PD::Ctr::AddDrawList(Stage::GetDrawDataTop(), false);
    // PD::Ctr::AddDrawList(Stage::GetDrawDataBottom(), true);
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
