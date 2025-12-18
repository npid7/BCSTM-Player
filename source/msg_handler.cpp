#include <msg_handler.hpp>

namespace D7 {
void MsgHandler::Elem::Render() {
  if (!pSettings) {
    return;
  }
  Rendered->DrawSolid();  // Draw Solid
  Rendered->SetFontScale(pSettings->FontScale);
  Rendered->DrawRectFilled(Pos, pSettings->MsgSize,
                           pSettings->ClrBackground);  // Background
  // Draw Line (if enabled)
  Rendered->PathAdd(Pos.Get() + PD::fvec2(2, 13));
  Rendered->PathAdd(Pos.Get() + PD::fvec2(pSettings->MsgSize.x - 4, 13));
  Rendered->PathStroke(PD::Color("#ffffff"));
  // Draw Header and Body
  Rendered->DrawText(Pos.Get() + PD::fvec2(2, 0), Header, pSettings->ClrText);
  PD::fvec2 tp = Pos.Get() + PD::fvec2(2, 13);
  PD::fvec2 bs = pSettings->MsgSize - PD::fvec2(4, 13);
  Rendered->DrawTextEx(tp, Body, pSettings->ClrText, LiTextFlags_Wrap, bs);
  // Debug stuff
  if (pSettings->Debug) {
    Rendered->DrawText(
        Pos.Get() + PD::fvec2(pSettings->MsgSize.x + 2, 0),
        std::format("{:.1f}\n[{:.0f}, {:.0f}]", Time, Pos.Get().x, Pos.Get().y),
        PD::Color("#ff00ff"));
  }
}

void MsgHandler::Elem::SlideIn() {
  if (!pSettings) {
    return;
  }
  // Simple EaseOutSine Slide in animation in 1 second
  Pos.From(
         PD::fvec2(-pSettings->MsgSize.x,
                   pSettings->VpH - pSettings->Padding - pSettings->MsgSize.y))
      .To(PD::fvec2(pSettings->Padding,
                    pSettings->VpH - pSettings->Padding - pSettings->MsgSize.y))
      .In(pSettings->FadeIn)
      .As(Pos.EaseOutSine);
}

void MsgHandler::Elem::SlideOut() {
  if (!pSettings) {
    return;
  }
  // Slide out with an EaseInSine in 0.5 seconds
  // oh and set ToBeRemoved to true fir deletion handler
  Pos.From(Pos)
      .To(PD::fvec2(-pSettings->MsgSize.x, Pos.Get().y))
      .In(pSettings->FadeOut)
      .As(Pos.EaseInSine);
  ToBeRemoved = true;
}

void MsgHandler::Elem::Move(int idx) {
  if (!pSettings) {
    return;
  }
  auto Pos2 = Pos;
  Pos2.Finish();
  // Move One Element + Padding Up in 0.5 seconds
  Pos.From(Pos)
      .To(PD::fvec2(Pos2.Get().x, pSettings->VpH - (pSettings->MsgSize.y +
                                                    pSettings->Padding) *
                                                       (idx + 1)))
      .In(pSettings->Move)
      .As(Pos.EaseInOutSine);
  MyIdx = idx;  // Set new index
}

void MsgHandler::Push(const std::string& head, const std::string& Body) {
  // Append a new element to the start
  pElems.emplace(pElems.begin(), head, Body, pFont, &pSettings);
}

void MsgHandler::Update(float d) {
  // Loop over every element
  for (size_t i = 0; i < pElems.size(); i++) {
    // Delete every element that is oos
    if (i > std::floor((pSettings.VpH / pSettings.MsgSize.y) + 1)) {
      pElems.erase(pElems.begin() + i);
      continue;
    }
    // Push up time by delta
    pElems[i].Time += d * 0.001;
    // Slide out element after MaxTime
    if (pElems[i].Time > pSettings.MaxTime && !pElems[i].ToBeRemoved) {
      pElems[i].SlideOut();
    }
    // Remove elem if removable (oos)
    if (pElems[i].Removable()) {
      pElems.erase(pElems.begin() + i);
      continue;  // and continue of course
    }
    // Move element to new index if not matching anymore
    if (pElems[i].MyIdx != i) {
      pElems[i].Move(i);
    }

    pElems[i].Pos.Update(d);  // Update the animation handler (PD::Tween Engine)
    pElems[i].Render();       // Render the element
    pDrawData->Merge(pElems[i].Rendered);  // Append to Universal Drawlist
  }
}

// Just return the drawlist ref
PD::Li::DrawList::Ref MsgHandler::GetDrawList() { return pDrawData; }
}  // namespace D7