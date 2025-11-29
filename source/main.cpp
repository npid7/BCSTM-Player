#include <bcstm/bcstmv2.hpp>
#include <bcstm_ctrl.hpp>
#include <bcstm_player.hpp>
#include <filebrowser.hpp>
#include <flex/flex.hpp>
#include <inspector_view.hpp>
#include <li_backend_c3d.hpp>
#include <pd.hpp>
#include <pd_ctr_ext.hpp>
#include <pd_hid_3ds.hpp>
#include <stagemgr.hpp>
#include <thread>

/**
 * Code is very unstable currently cause
 * 1. PD Ctr Extension is unfinished pre alpha
 * 2. Still using the bcstm/bcstmv2 impl which is based on the initial code
 * implementing the 1 to 8 channel support of bcstm_player.cpp
 * 3. bcstm_player.cpp is unused dead code due to it somehow crashes the fs
 * module on data block read
 * 4. inspector_view contains some bugs
 * 5. the flex ui engine is a small lib planned to be more simple then ui7 but
 * the way it works is bad
 * 6. The whole code around the stage_mgr is just in alpha state
 * 7. The whole code of bcstm_player 2.0.0 is completly hardcoded with extern
 * references through all the code and still very unfinished
 */

float Offset(float x) {
  float y = cos(x) * 42;
  return y - floor(y);
}

void Append(PD::LI::DrawList::Ref l, int index, fvec2 position, fvec2 size,
            float time) {
  float offset = Offset(index) * 62;
  float x_position =
      position.x + size.x / 8 * ((index % 11) - 1) + cos(offset + time) * 10;
  float y_position = position.y + size.y / 8 * (index / 11) + 40 +
                     sin(offset + time) * 10 + 30;
  float color_effect = 1 - exp(-(index / 11) / 3.0f);

  l->DrawTriangleFilled(
      fvec2(x_position, y_position), fvec2(x_position + 300, y_position + (90)),
      fvec2(x_position - 300, y_position + (90)),
      PD::Color(.94f - .17f * color_effect, .61f - .25f * color_effect,
                .36f + .38f * color_effect));
}
void BCSTM_Handler(BCSTM_Ctrl* ctrl) {
  while (true) {
    if (ctrl->pFileLoaded) {
#ifdef CTRFF_DECODE
      ctrl->plr.Stream();
#else
      ctrl->plr.Update();
#endif
    }
    if (ctrl->pRequests.Size() == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    } else {
      auto t = ctrl->pRequests.Front();
      if (t.req == BCSTM_Ctrl::OpenFile) {
        try {
          ctrl->plr.LoadFile(t.req_dat);
          /**
           * Set to true cause it would get set to false
           * if an error got thrown
           */
          ctrl->pFileLoaded = true;
        } catch (const std::exception& e) {
          std::cout << "BCSTM CTRL Error: " << e.what() << std::endl;
          ctrl->pFileLoaded = false;
        }
      } else if (t.req == BCSTM_Ctrl::CloseFile) {
        ctrl->plr.Stop();
      } else if (t.req == BCSTM_Ctrl::Stop) {
        ctrl->plr.Stop();
      } else if (t.req == BCSTM_Ctrl::Play) {
        ctrl->plr.Play();
      } else if (t.req == BCSTM_Ctrl::Pause) {
        ctrl->plr.Pause();
      } else if (t.req == BCSTM_Ctrl::KillThread) {
        break; /** Break the loop */
      }
      ctrl->pRequests.PopFront();
    }
  }
}

FileMgr::Ref Filebrowser;
Inspector::Ref FileInspector;
BCSTM_Ctrl bcstm_ctrl;

void BottomScreenBeta(PD::LI::DrawList::Ref l) {
  l->DrawRectFilled(0, fvec2(320, 240), 0xff222222);
  l->DrawRectFilled(5, fvec2(310, 20), 0xff111111);
  l->DrawRectFilled(7, fvec2(306, 16), 0xff222222);
  float scale = 0.f;
  D7::BcstmPlayer ll;

  if (bcstm_ctrl.plr.GetTotal() != 0) {
    scale =
        (float)bcstm_ctrl.plr.GetCurrent() / (float)bcstm_ctrl.plr.GetTotal();
  }
  l->DrawRectFilled(7, fvec2(306 * scale, 16), 0xff00ff00);
  l->DrawText(
      PD::fvec2(7, 28),
      std::format("Info:\n  Block Pos: {}/{}\n  Sample Rate: {}\n  Loop: {}\n  "
                  "Loop Start: "
                  "{}\n  Loop End: {}",
                  bcstm_ctrl.plr.GetCurrent(), bcstm_ctrl.plr.GetTotal(),
                  bcstm_ctrl.plr.GetSampleRate(), bcstm_ctrl.plr.IsLooping(),
                  bcstm_ctrl.plr.GetLoopStart(), bcstm_ctrl.plr.GetLoopEnd()),
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
  auto font = PD::LI::Font::New(PD::Ctr::GetContext().Gfx);
  font->LoadTTF("romfs:/fonts/ComicNeue.ttf");
  auto rl2 = PD::LI::DrawList::New(PD::Ctr::GetWhiteTex());
  auto rl3 = PD::LI::DrawList::New(PD::Ctr::GetWhiteTex());
  rl3->SetFont(font);
  Filebrowser =
      FileMgr::New(PD::Ctr::GetContext().Inp, PD::Ctr::GetWhiteTex(), font);
  FileInspector =
      Inspector::New(PD::Ctr::GetContext().Inp, PD::Ctr::GetWhiteTex(), font);
  Stage::Goto(Filebrowser);

  PD::Timer time;
  while (PD::Ctr::ContextUpdate()) {
    time.Update();
    rl2->SetFont(font);

    rl2->DrawRectFilled(fvec2(0, 0), fvec2(400, 240), 0xff64c9fd);
    for (int i = 0; i < 44; i++)
      Append(rl2, i, fvec2(0, 0), fvec2(400, 240), (float)time.GetSeconds());
    BottomScreenBeta(rl3);

    Stage::DoUpdate();
    PD::Ctr::AddDrawList(rl2, false);
    PD::Ctr::AddDrawList(rl3, true);
    PD::Ctr::AddDrawList(Stage::GetDrawDataTop(), false);
    // PD::Ctr::AddDrawList(Stage::GetDrawDataBottom(), true);
    if (PD::Ctr::GetContext().Inp->IsDown(PD::Ctr::GetContext().Inp->Start)) {
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
