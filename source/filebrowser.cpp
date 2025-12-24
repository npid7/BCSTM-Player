#include <filebrowser.hpp>
#include <stages.hpp>

void FileMgr::Update() {
  delta.Update();
  cursor.Update(delta.GetSeconds() * 1000.f);
  delta.Reset();

  for (int i = 0; i < 12; i++) {
    Top->Rect()
        .SetPos(PD::fvec2(0, 18 + 17 * i))
        .SetSize(PD::fvec2(400, 17))
        .SetColor(((i % 2) == 0) ? GetTheme().ListEven : GetTheme().ListOdd);
  }
  if (pShowHelp) {
    Top->Text(
           "Controls:\nUp -> Go 1 Entry Up\nDown -> Go 1 Entry Down\nLeft -> "
           "Go 5 "
           "Entries Up\nRight -> Go 5 Entries Down\nA -> Go into Directory / "
           "Play "
           "BCSTM File\nB -> Go Back\nX -> Open .bcstm File in File "
           "Inspector\nStart -> Exit App")
        .SetPos(PD::fvec2(5, 18))
        .SetColor(GetTheme().Text);
  } else {
    Top->Rect()
        .SetPos(cursor)
        .SetSize(PD::fvec2(400, 17))
        .SetColor(GetTheme().Selector);
    for (int i = 0; i < int(list.size() > 12 ? 12 : list.size()); i++) {
      auto tmp = std::filesystem::path(list[sp + i].Path);
      if (!tmp.extension().compare(".bcstm")) {
        Top->Image(pIcnAudio).SetPos(PD::fvec2(2, 18 + 17 * i));
      } else {
        Top->Image(list[sp + i].Dir ? pIcnFolder : pIcnFile)
            .SetPos(PD::fvec2(2, 18 + 17 * i));
      }
      Top->Text(list[sp + i].Name)
          .SetPos(PD::fvec2(21, 18 + 17 * i))
          .SetColor(GetTheme().Text);
    }
  }
  Top->Rect().SetColor(GetTheme().Header).SetPos(0).SetSize(PD::fvec2(400, 18));
  Top->Text(Lang.Get("HEAD_FILEMANAGER"))
      .SetPos(PD::fvec2(5, 1))
      .SetColor(GetTheme().Text);
  Top->Rect()
      .SetPos(PD::fvec2(0, 222))
      .SetSize(PD::fvec2(400, 18))
      .SetColor(GetTheme().Footer);
  Top->Text(cPath).SetPos(PD::fvec2(5, 223)).SetColor(GetTheme().Text);
  if (list.size() > 12) {
    float rect_h = (12.f / (float)list.size()) * 204.f;
    /** Make sure the rect is still visible */
    rect_h = std::clamp<float>(rect_h, 10.f, 204.f);
    float rect_pos =
        18.f + ((float)sp / (float)(list.size() - 12)) * (204.f - rect_h);
    Top->Rect()
        .SetPos(PD::fvec2(396, rect_pos))
        .SetSize(PD::fvec2(4, rect_h))
        .SetColor(GetTheme().Slider);
  }
  if (!pShowHelp) {
    if (PD::Hid::IsUp(PD::Hid::Key::Down) &&
        sp + cursor.pIndex < (int)list.size() - 1) {
      if (cursor.pIndex == 11) {
        sp++;
      } else {
        cursor++;
      }
    }
    if (PD::Hid::IsUp(PD::Hid::Key::Right) &&
        sp + cursor.pIndex + 5 < (int)list.size()) {
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
      auto FSE = list[cursor.pIndex + sp];
      if (FSE.Dir) {
        pLastPos.push(std::make_pair(sp, cursor.GetIndex()));
        cursor.SetIndex(0);
        sp = 0;
        ScanDir(FSE.Path);
      } else if (FSE.Name.find(".bcstm") != FSE.Name.npos) {
        ctrl.DoRequest(ctrl.Stop);
        ctrl.DoRequest(ctrl.OpenFile, FSE.Path);
        ctrl.DoRequest(ctrl.Play);
      }
    }

    if (PD::Hid::IsDown(PD::Hid::Key::B)) {
      if (cPath != "sdmc:/") {
        cPath = std::filesystem::path(cPath).parent_path().string();
        if (cPath == "sdmc:") {
          cPath += "/";
        }
        if (!pLastPos.empty()) {
          sp = pLastPos.top().first;
          cursor.SetIndex(pLastPos.top().second);
          pLastPos.pop();
        } else {
          sp = 0;
          cursor.SetIndex(0);
        }
        ScanDir(cPath);
      }
    }

    if (PD::Hid::IsDown(PD::Hid::Key::X)) {
      auto FSE = list[cursor.pIndex + sp];
      if (FSE.Name.find(".bcstm") != FSE.Name.npos) {
        // THe hacky way
        auto FileInspector = Inspector::New(this->Top->pFont);
        FileInspector->ReadFile(FSE.Path);
        Goto(FileInspector);
      } else if (FSE.Name.find(".bcwav") != FSE.Name.npos) {
        auto FileInspectorBCWAV = InspectorBCWAV::New(this->Top->pFont);
        FileInspectorBCWAV->ReadFile(FSE.Path);
        Goto(FileInspectorBCWAV);
      }
    }
  }

  pShowHelp = PD::Hid::IsHeld(PD::Hid::Key::Select);
}

void FileMgr::SortList(std::vector<FSEntry>& l) {
  std::sort(l.begin(), l.end(), [](FSEntry& a, FSEntry& b) -> bool {
    if (!a.Dir && b.Dir) {
      return false;
    }
    if (a.Dir && !b.Dir) {
      return true;
    }
    std::string la = a.Name;
    std::string lb = b.Name;
    std::transform(la.begin(), la.end(), la.begin(),
                   [](char c) { return std::tolower(c); });
    std::transform(lb.begin(), lb.end(), lb.begin(),
                   [](char c) { return std::tolower(c); });
    return la.compare(lb) < 0;
  });
}

void FileMgr::BackgroundListUpdate(std::vector<FSEntry>& l,
                                   const std::string& path) {
  l.clear();
  for (auto& it : std::filesystem::directory_iterator(path)) {
    l.push_back(
        {it.is_directory(), it.path().string(), it.path().filename().string()});
    std::this_thread::sleep_for(std::chrono::microseconds(5));
  }
  SortList(l);
}

void FileMgr::ScanDir(const std::string& path) {
  cPath = path;
  BackgroundListUpdate(list, path);
  // std::thread tmp([&]() { BackgroundListUpdate(list, path); });
  // tmp.detach();
}