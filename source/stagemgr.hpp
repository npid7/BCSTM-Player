#pragma once

#include <flex/flex.hpp>
#include <pd.hpp>

class Stage {
 public:
  Stage(PD::Hid::Ref inp, PD::LI::Texture::Ref wp, PD::LI::Font::Ref f) {
    Top = PD::Flex::Context::New(wp, f, inp);
    Bottom = PD::Flex::Context::New(wp, f, inp);
    Inp = inp;
  }
  virtual ~Stage() {}
  PD_SMART_CTOR(Stage);

  PD::Flex::Context::Ref Top;
  PD::Flex::Context::Ref Bottom;
  PD::Hid::Ref Inp;

  virtual void Update() {}

  static void Goto(Stage::Ref s) { pStages.Push(s); }
  static void Back() { pStages.Pop(); }
  static void DoUpdate() {
    pStages.Top()->Top->pDrawList->Clear();
    pStages.Top()->Bottom->pDrawList->Clear();
    pStages.Top()->Update();
    pStages.Top()->Top->Update();
    pStages.Top()->Bottom->Update();
  };
  static PD::LI::DrawList::Ref GetDrawDataTop() {
    return pStages.Top()->Top->pDrawList;
  }
  static PD::LI::DrawList::Ref GetDrawDataBottom() {
    return pStages.Top()->Bottom->pDrawList;
  }

  static PD::Stack<Stage::Ref> pStages;
};
