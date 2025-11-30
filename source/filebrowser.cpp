#include <bcstm_ctrl.hpp>
#include <filebrowser.hpp>
#include <stages.hpp>

void FileMgr::Update() {
  delta.Update();
  cursor.Update(delta.GetSeconds() * 1000.f);
  delta.Reset();

  for (int i = 0; i < 12; i++) {
    Top->Rect()
        .SetPos(fvec2(0, 18 + 17 * i))
        .SetSize(fvec2(400, 17))
        .SetColor(((i % 2) == 0) ? PD::Color("#222222aa")
                                 : PD::Color("#333333aa"));
  }
  if (pShowHelp) {
    Top->Text(
           "Controls:\nUp -> Go 1 Entry Up\nDown -> Go 1 Entry Down\nLeft -> "
           "Go 5 "
           "Entries Up\nRight -> Go 5 Entries Down\nA -> Go into Directory / "
           "Play "
           "BCSTM File\nB -> Go Back\nX -> Open .bcstm File in File "
           "Inspector\nStart -> Exit App")
        .SetPos(fvec2(5, 18))
        .SetColor(PD::Colors::White);
  } else {
    Top->Rect()
        .SetPos(cursor)
        .SetSize(fvec2(400, 17))
        .SetColor(PD::Color("#222222cc"));
    for (int i = 0; i < int(list.size() > 12 ? 12 : list.size()); i++) {
      Top->Text(list[sp + i].Name)
          .SetPos(fvec2(5, 18 + 17 * i))
          .SetColor(PD::Colors::White);
    }
  }
  Top->Rect().SetColor(DesignerHeader).SetPos(0).SetSize(fvec2(400, 18));
  Top->Text("BCSTM-Player -> Filebrowser")
      .SetPos(fvec2(5, 1))
      .SetColor(PD::Colors::White);
  Top->Rect()
      .SetPos(fvec2(0, 222))
      .SetSize(fvec2(400, 18))
      .SetColor(DesignerHeader);
  Top->Text(cPath).SetPos(fvec2(5, 223)).SetColor(PD::Colors::White);
  if (list.size() > 12) {
    float rect_h = (12.f / (float)list.size()) * 204.f;
    /** Make sure the rect is still visible */
    rect_h = std::clamp<float>(rect_h, 10.f, 204.f);
    float rect_pos =
        18.f + ((float)sp / (float)(list.size() - 12)) * (204.f - rect_h);
    Top->Rect()
        .SetPos(fvec2(396, rect_pos))
        .SetSize(fvec2(4, rect_h))
        .SetColor(PD::Colors::DarkGray);
  }
  if (!pShowHelp) {
    if (Inp->IsUp(Inp->Down) && sp + cursor.pIndex < (int)list.size() - 1) {
      if (cursor.pIndex == 11) {
        sp++;
      } else {
        cursor++;
      }
    }
    if (Inp->IsUp(Inp->Right) && sp + cursor.pIndex + 5 < (int)list.size()) {
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
      auto FSE = list[cursor.pIndex + sp];
      if (FSE.Dir) {
        pLastPos.Push(PD::Pair<int, int>(sp, cursor.GetIndex()));
        cursor.SetIndex(0);
        sp = 0;
        ScanDir(FSE.Path);
      } else if (FSE.Name.find(".bcstm") != FSE.Name.npos) {
        bcstm_ctrl.DoRequest(bcstm_ctrl.Stop);
        bcstm_ctrl.DoRequest(bcstm_ctrl.OpenFile, FSE.Path);
        bcstm_ctrl.DoRequest(bcstm_ctrl.Play);
      }
    }

    if (Inp->IsDown(Inp->B)) {
      if (cPath != "sdmc:/") {
        cPath = std::filesystem::path(cPath).parent_path().string();
        if (cPath == "sdmc:") {
          cPath += "/";
        }
        if (!pLastPos.IsEmpty()) {
          sp = pLastPos.Top().First;
          cursor.SetIndex(pLastPos.Top().Second);
          pLastPos.Pop();
        } else {
          sp = 0;
          cursor.SetIndex(0);
        }
        ScanDir(cPath);
      }
    }

    if (Inp->IsDown(Inp->X)) {
      auto FSE = list[cursor.pIndex + sp];
      if (FSE.Name.find(".bcstm") != FSE.Name.npos) {
        FileInspector->ReadFile(FSE.Path);
        Goto(FileInspector);
      }
    }

    if (Inp->IsDown(Inp->L)) {
      Goto(Settings);
    }
  }

  pShowHelp = Inp->IsHeld(Inp->Select);
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