#include <bcstm_ctrl.hpp>
#include <settings.hpp>
#include <stages.hpp>

void Settings::Update() {
  delta.Update();
  cursor.Update(delta.GetSeconds() * 1000.f);
  delta.Reset();

  for (int i = 0; i < 12; i++) {
    Top->Rect()
        .SetPos(PD::fvec2(0, 18 + 17 * i))
        .SetSize(PD::fvec2(200, 17))
        .SetColor(((i % 2) == 0) ? PD::Color("#222222aa")
                                 : PD::Color("#333333aa"));
    Top->Rect()
        .SetPos(PD::fvec2(200, 18 + 17 * i))
        .SetSize(PD::fvec2(200, 17))
        .SetColor(((i % 2) != 0) ? PD::Color("#222222aa")
                                 : PD::Color("#333333aa"));
  }
  Top->Rect()
      .SetPos(cursor)
      .SetSize(PD::fvec2(400, 17))
      .SetColor(PD::Color("#222222cc"));
  for (int i = 0; i < int(pDL.size() > 12 ? 12 : pDL.size()); i++) {
    Top->Text(pDL.at(sp + i)->First)
        .SetPos(PD::fvec2(5, 18 + 17 * i))
        .SetColor(White);
    Top->Text(pDL.at(sp + i)->Second)
        .SetPos(PD::fvec2(205, 18 + 17 * i))
        .SetColor(White);
  }
  Top->Rect().SetColor(DesignerHeader).SetPos(0).SetSize(PD::fvec2(400, 18));
  Top->Text(Lang.Get("HEAD_SETTINGS")).SetPos(PD::fvec2(5, 1)).SetColor(White);
  Top->Rect()
      .SetPos(PD::fvec2(0, 222))
      .SetSize(PD::fvec2(400, 18))
      .SetColor(DesignerHeader);
  if (pDL.size() > 12) {
    float rect_h = (12.f / (float)pDL.size()) * 204.f;
    /** Make sure the rect is still visible */
    rect_h = std::clamp<float>(rect_h, 10.f, 204.f);
    float rect_pos =
        18.f + ((float)sp / (float)(pDL.size() - 12)) * (204.f - rect_h);
    Top->Rect()
        .SetPos(PD::fvec2(396, rect_pos))
        .SetSize(PD::fvec2(4, rect_h))
        .SetColor(DarkGray);
  }

  if (PD::Hid::IsUp(PD::Hid::Key::Down) &&
      sp + cursor.pIndex < (int)pDL.size() - 1) {
    if (cursor.pIndex == 11) {
      sp++;
    } else {
      cursor++;
    }
  }
  if (PD::Hid::IsUp(PD::Hid::Key::Right) &&
      sp + cursor.pIndex + 5 < (int)pDL.size()) {
    if (cursor.pIndex == 11) {
      sp += 5;
    } else {
      if (cursor.pIndex + 5 > 11) {
        sp += (cursor.pIndex - 11 + 5);
        cursor.SetIndex(11);
      } else {
        cursor += 5;
      }
    }
  }
  if (PD::Hid::IsUp(PD::Hid::Key::Up) && cursor.pIndex + sp > 0) {
    if (cursor.pIndex == 0) {
      sp--;
    } else {
      cursor--;
    }
  }
  if (PD::Hid::IsUp(PD::Hid::Key::Left) && sp + cursor.pIndex - 5 >= 0) {
    if (cursor.pIndex == 0) {
      sp -= 5;
    } else {
      if (cursor.pIndex - 5 < 0) {
        sp -= 5 - cursor.pIndex;
        cursor.SetIndex(0);
      } else {
        cursor -= 5;
      }
    }
  }

  if (PD::Hid::IsDown(PD::Hid::Key::A)) {
    auto FSE = pDL.at(cursor.pIndex + sp);
    if (FSE->pFunc) {
      FSE->pFunc(FSE->Second);
    } else if (FSE->SubData.size() != 0) {
      pLastPos.push(std::make_pair(sp, cursor.GetIndex()));
      cursor.SetIndex(0);
      sp = 0;
      pStack.push(pDL);
      pDL = FSE->SubData;
    }
  }

  if (PD::Hid::IsDown(PD::Hid::Key::B)) {
    if (!pLastPos.empty()) {
      sp = pLastPos.top().first;
      cursor.SetIndex(pLastPos.top().second);
      pLastPos.pop();
    } else {
      sp = 0;
      cursor.SetIndex(0);
    }
    if (pStack.empty()) {
      Back();
    } else {
      pDL = pStack.top();
      pStack.pop();
    }
  }
}

Settings::TabEntry::Ref Settings::MakeEntry(const std::string& id,
                                            const std::string& val) {
  auto e = TabEntry::New();
  e->First = id;
  e->Second = val;
  return e;
}

Settings::TabEntry::Ref MakeInfo() {
  auto e = Settings::TabEntry::New();
  std::vector<Settings::TabEntry::Ref> pData;
  pData.push_back(Settings::MakeEntry("Version", VERSION));
  pData.push_back(Settings::MakeEntry("Commit", GIT_COMMIT));
  pData.push_back(Settings::MakeEntry("Banch", GIT_BRANCH));
  pData.push_back(Settings::MakeEntry(
      "C++", std::format("{} ({}-{})", (__cplusplus / 100) % 100,
                         __cplusplus / 100, __cplusplus % 100)));
  pData.push_back(
      Settings::MakeEntry("Compiler", PD::Strings::GetCompilerVersion()));
  pData.push_back(Settings::MakeEntry("Build", __DATE__ " " __TIME__));
  pData.push_back(Settings::MakeEntry("RTTI",
#ifdef __RTTI
                                      "ON"
#else
                                      "OFF"
#endif
                                      ));
  pData.push_back(Settings::MakeEntry("Exceptions",
#ifdef __EXCEPTIONS
                                      "ON"
#else
                                      "OFF"
#endif
                                      ));
  e->First = "Show Info";
  e->SubData = pData;
  return e;
}

Settings::TabEntry::Ref MakeCredits() {
  auto e = Settings::TabEntry::New();
  std::vector<Settings::TabEntry::Ref> pData;
  pData.push_back(Settings::MakeEntry("tobid7", "Lead deceloper, assets"));
  pData.push_back(Settings::MakeEntry("", "palladium, ctrff"));
  pData.push_back(Settings::MakeEntry("devkitpro", "libctru, citro3d"));
  pData.push_back(Settings::MakeEntry("cheuble", "Original BCSTMV1-Decoder"));
  pData.push_back(Settings::MakeEntry("3DBrew", "BCSTM Documentation"));
  pData.push_back(Settings::MakeEntry("crozynski", "ComicNeue Font"));

  e->First = "Credits";
  e->SubData = pData;
  return e;
}

void Settings::Init() {
  List.clear();

  List.push_back(MakeInfo());
  List.push_back(MakeCredits());
  List.push_back(TabEntry::New(
      "Decoder", bcstm_ctrl.player->GetName(), [=, this](std::string& s) {
        s = bcstm_ctrl.player->GetName() == "CTRFFDec" ? "BCSTMV2" : "CTRFFDec";
        bcstm_ctrl.DoRequest(bcstm_ctrl.SwitchDec);
      }));
  pDL = List;
}