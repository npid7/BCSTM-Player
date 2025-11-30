#include <bcstm_ctrl.hpp>
#include <settings.hpp>

void Settings::Update() {
  delta.Update();
  cursor.Update(delta.GetSeconds() * 1000.f);
  delta.Reset();

  for (int i = 0; i < 12; i++) {
    Top->Rect()
        .SetPos(fvec2(0, 18 + 17 * i))
        .SetSize(fvec2(200, 17))
        .SetColor(((i % 2) == 0) ? PD::Color("#222222aa")
                                 : PD::Color("#333333aa"));
    Top->Rect()
        .SetPos(fvec2(200, 18 + 17 * i))
        .SetSize(fvec2(200, 17))
        .SetColor(((i % 2) != 0) ? PD::Color("#222222aa")
                                 : PD::Color("#333333aa"));
  }
  Top->Rect()
      .SetPos(cursor)
      .SetSize(fvec2(400, 17))
      .SetColor(PD::Color("#222222cc"));
  for (int i = 0; i < int(pDL.size() > 12 ? 12 : pDL.size()); i++) {
    Top->Text(pDL.at(sp + i)->First)
        .SetPos(fvec2(5, 18 + 17 * i))
        .SetColor(PD::Colors::White);
    Top->Text(pDL.at(sp + i)->Second)
        .SetPos(fvec2(205, 18 + 17 * i))
        .SetColor(PD::Colors::White);
  }
  Top->Rect().SetColor(DesignerHeader).SetPos(0).SetSize(fvec2(400, 18));
  Top->Text("BCSTM-Player -> Settings")
      .SetPos(fvec2(5, 1))
      .SetColor(PD::Colors::White);
  Top->Rect()
      .SetPos(fvec2(0, 222))
      .SetSize(fvec2(400, 18))
      .SetColor(DesignerHeader);
  if (pDL.size() > 12) {
    float rect_h = (12.f / (float)pDL.size()) * 204.f;
    /** Make sure the rect is still visible */
    rect_h = std::clamp<float>(rect_h, 10.f, 204.f);
    float rect_pos =
        18.f + ((float)sp / (float)(pDL.size() - 12)) * (204.f - rect_h);
    Top->Rect()
        .SetPos(fvec2(396, rect_pos))
        .SetSize(fvec2(4, rect_h))
        .SetColor(PD::Colors::DarkGray);
  }

  if (Inp->IsUp(Inp->Down) && sp + cursor.pIndex < (int)pDL.size() - 1) {
    if (cursor.pIndex == 11) {
      sp++;
    } else {
      cursor++;
    }
  }
  if (Inp->IsUp(Inp->Right) && sp + cursor.pIndex + 5 < (int)pDL.size()) {
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
  if (Inp->IsUp(Inp->Up) && cursor.pIndex + sp > 0) {
    if (cursor.pIndex == 0) {
      sp--;
    } else {
      cursor--;
    }
  }
  if (Inp->IsUp(Inp->Left) && sp + cursor.pIndex - 5 >= 0) {
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

  if (Inp->IsDown(Inp->A)) {
    auto FSE = pDL.at(cursor.pIndex + sp);
    if (FSE->pFunc) {
      FSE->pFunc(FSE->Second);
    } else if (FSE->SubData.size() != 0) {
      pLastPos.Push(PD::Pair<int, int>(sp, cursor.GetIndex()));
      cursor.SetIndex(0);
      sp = 0;
      pStack.Push(pDL);
      pDL = FSE->SubData;
    }
  }

  if (Inp->IsDown(Inp->B)) {
    if (!pLastPos.IsEmpty()) {
      sp = pLastPos.Top().First;
      cursor.SetIndex(pLastPos.Top().Second);
      pLastPos.Pop();
    } else {
      sp = 0;
      cursor.SetIndex(0);
    }
    if (pStack.IsEmpty()) {
      Back();
    } else {
      pDL = pStack.Top();
      pStack.Pop();
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
        s = bcstm_ctrl.player->GetName() == "BCSTMV2" ? "CTRFF (WIP)"
                                                      : "BCSTMV2";
        bcstm_ctrl.DoRequest(bcstm_ctrl.SwitchDec);
      }));
  pDL = List;
}