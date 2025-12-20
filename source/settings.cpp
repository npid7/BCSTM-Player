#include <bcstm_ctrl.hpp>
#include <msg_handler.hpp>
#include <pd_ctr_ext.hpp>
#include <settings.hpp>
#include <stages.hpp>

extern D7::MsgHandler::Ref MsgHnd;

void Settings::Update() {
  delta.Update();
  cursor.Update(delta.GetSeconds() * 1000.f);
  delta.Reset();

  for (int i = 0; i < 12; i++) {
    Top->Rect()
        .SetPos(PD::fvec2(0, 18 + 17 * i))
        .SetSize(PD::fvec2(200, 17))
        .SetColor(((i % 2) == 0) ? gTheme.ListEven : gTheme.ListOdd);
    Top->Rect()
        .SetPos(PD::fvec2(200, 18 + 17 * i))
        .SetSize(PD::fvec2(200, 17))
        .SetColor(((i % 2) != 0) ? gTheme.ListEven : gTheme.ListOdd);
  }
  Top->Rect()
      .SetPos(cursor)
      .SetSize(PD::fvec2(400, 17))
      .SetColor(gTheme.Selector);
  for (int i = 0; i < int(pDL.size() > 12 ? 12 : pDL.size()); i++) {
    Top->Text(pDL.at(sp + i)->First)
        .SetPos(PD::fvec2(5, 18 + 17 * i))
        .SetColor(gTheme.Text);
    Top->Text(pDL.at(sp + i)->Second)
        .SetPos(PD::fvec2(205, 18 + 17 * i))
        .SetColor(gTheme.Text);
  }
  Top->Rect().SetColor(gTheme.Header).SetPos(0).SetSize(PD::fvec2(400, 18));
  Top->Text(Lang.Get("HEAD_SETTINGS"))
      .SetPos(PD::fvec2(5, 1))
      .SetColor(gTheme.Text);
  Top->Rect()
      .SetPos(PD::fvec2(0, 222))
      .SetSize(PD::fvec2(400, 18))
      .SetColor(gTheme.Footer);
  Top->Text(Lang.Get("TPWMR")).SetPos(PD::fvec2(5, 223)).SetColor(gTheme.Text);
  if (pDL.size() > 12) {
    float rect_h = (12.f / (float)pDL.size()) * 204.f;
    /** Make sure the rect is still visible */
    rect_h = std::clamp<float>(rect_h, 10.f, 204.f);
    float rect_pos =
        18.f + ((float)sp / (float)(pDL.size() - 12)) * (204.f - rect_h);
    Top->Rect()
        .SetPos(PD::fvec2(396, rect_pos))
        .SetSize(PD::fvec2(4, rect_h))
        .SetColor(gTheme.Slider);
  }
  Bottom->Rect()
      .SetPos(0)
      .SetSize(PD::fvec2(320, 240))
      .SetColor(gTheme.Background);

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
      if (pCfg.Updated()) {
        try {
          pCfg.Save("sdmc:/3ds/BCSTM-Player/config.json");
        } catch (const std::runtime_error& e) {
          MsgHnd->Push("Config Save Error", e.what());
        }
      }
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
  e->First = Lang.Get("SHOW_INFO");
  e->SubData = pData;
  return e;
}

Settings::TabEntry::Ref Settings::MakeCredits() {
  auto e = TabEntry::New();
  std::vector<TabEntry::Ref> pData;
  pData.push_back(MakeEntry("tobid7", "Lead deceloper, assets"));
  pData.push_back(MakeEntry("", "palladium, ctrff"));
  pData.push_back(MakeEntry("devkitpro", "libctru, citro3d"));
  pData.push_back(MakeEntry("cheuble", "Original BCSTMV1-Decoder"));
  pData.push_back(MakeEntry("3DBrew", "BCSTM Documentation"));
  auto pSpecial = MakeLicenseText("romfs:/license/ComicNeue.txt");
  pSpecial->First = "crozynski";
  pSpecial->Second = "ComicNeue Font";
  pData.push_back(pSpecial);

  e->First = Lang.Get("CREDITS");
  e->SubData = pData;
  return e;
}

Settings::TabEntry::Ref Settings::MakeLang() {
  auto e = TabEntry::New();
  std::vector<TabEntry::Ref> pData;
  pData.push_back(TabEntry::New("System", "", [=, this](std::string& s) {
    try {
      Lang.Load("romfs:/lang/" + PD::Ctr::GetSystemLanguage() + ".json");
    } catch (const std::runtime_error& e) {
      Lang.Load("romfs:/lang/de.json");
      MsgHnd->Push("Lang Error:", e.what());
    }
    pCfg.Set("last_lang", "sys");
  }));
  for (auto& it : std::filesystem::directory_iterator("romfs:/lang")) {
    D7::Lang tmp;
    tmp.Load(it.path().string());
    pData.push_back(TabEntry::New(tmp.Id(), tmp.Name() + " - " + tmp.Author(),
                                  [=, this](std::string& s) {
                                    Lang.Load(it.path().string());
                                    pCfg.Set("last_lang", tmp.Id());
                                    Init();
                                  }));
  }

  e->First = Lang.Get("LANGUAGE");
  e->Second = Lang.Name();
  e->SubData = pData;
  return e;
}

Settings::TabEntry::Ref Settings::MakeThemes() {
  auto e = Settings::TabEntry::New();
  std::vector<Settings::TabEntry::Ref> pData;
  for (auto& it :
       std::filesystem::directory_iterator("sdmc:/3ds/BCSTM-Player/themes")) {
    pData.push_back(Settings::TabEntry::New(
        it.path().filename().stem().string(), "", [=, this](std::string& s) {
          gTheme.Load(it.path().string());
          pCfg.Set("last_theme", it.path().filename().stem().string());
        }));
  }

  e->First = Lang.Get("THEMES") + " (Beta)";
  e->SubData = pData;
  return e;
}

Settings::TabEntry::Ref Settings::MakeLicenseText(const std::string& path) {
  auto e = Settings::TabEntry::New();
  std::vector<Settings::TabEntry::Ref> pData;
  std::fstream iff(path, std::ios::in);
  if (iff) {
    std::string theline;
    while (std::getline(iff, theline)) {
      std::istringstream is(theline);
      std::string line;
      std::string tmp;
      while (is >> tmp) {
        if (Top->pFont
                ->GetTextBounds(line + " " + tmp, Top->pDrawList->pFontScale)
                .x < 390) {
          if (!line.empty()) {
            line += " ";
          }
          line += tmp;
        } else {
          pData.push_back(MakeEntry(line, ""));
          line = tmp;
        }
      }
      pData.push_back(MakeEntry(line, ""));
    }
  }
  iff.close();
  e->First = Lang.Get("LICENSE");
  e->SubData = pData;
  return e;
}

Settings::TabEntry::Ref Settings::MakeClock() {
  auto e = Settings::TabEntry::New();
  std::vector<Settings::TabEntry::Ref> pData;
  pData.push_back(TabEntry::New(
      Lang.Get("24HRS"), std::format("{}", pCfg.Get<bool>("clock_fmt24")),
      [=, this](std::string& s) {
        pCfg.Set("clock_fmt24", !pCfg.Get<bool>("clock_fmt24"));
        s = std::format("{}", pCfg.Get<bool>("clock_fmt24"));
      }));
  pData.push_back(TabEntry::New(
      Lang.Get("SHOWSECONDS"),
      std::format("{}", pCfg.Get<bool>("clock_seconds")),
      [=, this](std::string& s) {
        pCfg.Set("clock_seconds", !pCfg.Get<bool>("clock_seconds"));
        s = std::format("{}", pCfg.Get<bool>("clock_seconds"));
      }));
  e->First = Lang.Get("CLOCK");
  e->SubData = pData;
  return e;
}

void Settings::Init() {
  List.clear();
  sp = 0;
  while (!pStack.empty()) {
    pStack.pop();
  }
  while (!pLastPos.empty()) {
    pLastPos.pop();
  }

  List.push_back(TabEntry::New(
      "Decoder", bcstm_ctrl.player->GetName(), [=, this](std::string& s) {
        s = bcstm_ctrl.player->GetName() == "CTRFFDec" ? "BCSTMV2" : "CTRFFDec";
        bcstm_ctrl.DoRequest(bcstm_ctrl.SwitchDec);
      }));
  List.push_back(MakeLang());
  List.push_back(MakeThemes());
  List.push_back(MakeClock());
  List.push_back(TabEntry::New(
      Lang.Get("RD7TFBG"), std::format("{}", pCfg.Get<bool>("rd7tfbg")),
      [=, this](std::string& s) {
        pCfg.Set("rd7tfbg", !pCfg.Get<bool>("rd7tfbg"));
        s = std::format("{}", pCfg.Get<bool>("rd7tfbg"));
      }));
  List.push_back(MakeCredits());
  List.push_back(MakeInfo());
  pDL = List;
}