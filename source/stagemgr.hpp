#pragma once

#include <flex/flex.hpp>
#include <palladium>

class Stage {
 public:
  Stage(PD::Li::Font::Ref f) {
    Top = PD::Flex::Context::New(f);
    Bottom = PD::Flex::Context::New(f);
  }
  virtual ~Stage() {}
  PD_SHARED(Stage);

  PD::Flex::Context::Ref Top;
  PD::Flex::Context::Ref Bottom;

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
  static PD::Li::DrawList::Ref GetDrawDataTop() {
    return pStages.Top()->Top->pDrawList;
  }
  static PD::Li::DrawList::Ref GetDrawDataBottom() {
    return pStages.Top()->Bottom->pDrawList;
  }

  static PD::Stack<Stage::Ref> pStages;
};
