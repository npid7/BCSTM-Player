#pragma once

#include <palladium>

class Progressbar : public PD::UI7::Container {
 public:
  /**
   * Constructor for the Progresbar Object
   */
  Progressbar(float v) {
    this->pVal = v;
    SetSize(PD::fvec2(310, 20));
  }
  ~Progressbar() = default;

  PD_SHARED(Progressbar);

  /**
   * Override for the Rendering Handler
   * @note This function is usally called by Menu::Update
   * */
  void Draw() override {
    auto p = FinalPos();
    list->DrawRectFilled(
        p, GetSize(),
        this->GetIO()->Theme->Get(UI7Color_FrameBackgroundHovered));
    list->DrawRectFilled(p + PD::fvec2(2), GetSize() - 4,
                         this->GetIO()->Theme->Get(UI7Color_FrameBackground));

    list->DrawRectFilled(p + PD::fvec2(2),
                         PD::fvec2((GetSize().x - 4) * pVal, GetSize().y - 4),
                         this->GetIO()->Theme->Get(UI7Color_Progressbar));
  }

 private:
  float pVal;
};