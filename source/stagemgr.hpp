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

  static void Goto(Stage::Ref s) { pStages.push(s); }
  static void Back() { pStages.pop(); }
  static void DoUpdate() {
    pStages.top()->Top->pDrawList->Clear();
    pStages.top()->Bottom->pDrawList->Clear();
    pStages.top()->Update();
    pStages.top()->Top->Update();
    pStages.top()->Bottom->Update();
  };
  static PD::Li::DrawList::Ref GetDrawDataTop() {
    return pStages.top()->Top->pDrawList;
  }
  static PD::Li::DrawList::Ref GetDrawDataBottom() {
    return pStages.top()->Bottom->pDrawList;
  }
  static void SetTheme(Theme& t) { pTheme = &t; }
  static Theme& GetTheme() { return *pTheme; }

  static std::stack<Stage::Ref> pStages;
  static Theme* pTheme;
};
