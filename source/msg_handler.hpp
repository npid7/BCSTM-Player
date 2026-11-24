#pragma once

#include <flex/container.hpp>
#include <memory>
#include <palladium>
#include <string>

namespace D7 {
class MsgHandler {
 public:
  struct Settings {
    Settings() {}
    PD::fvec2 MsgSize = PD::fvec2(160, 54);
    PD::u32 ClrBackground = PD::Color("#333333cc");
    PD::u32 ClrText = PD::Color("#ffffff");
    float FontScale = 0.6f;
    float MaxTime = 5.f;
    float FadeIn = 1.f;
    float Move = 0.5f;
    float FadeOut = 0.5f;
    float Padding = 5.f;
    float VpH = 240.f;
    bool Debug = false;
    int BaseLayer = 3;
  };

  struct Elem {
    Elem(PD::Li::Font* f = nullptr, Settings* s = nullptr) {
      Rendered.SetFont(f);
      pSettings = s;
    }
    Elem(const std::string& h, const std::string& b, PD::Li::Font* f = nullptr,
         Settings* s = nullptr) {
      pSettings = s;
      Header = h;
      Body = b;
      Rendered.SetFont(f);
      SlideIn();
    }
    void SlideIn();
    void SlideOut();
    void Move(int idx);
    bool Removable() const { return ToBeRemoved && Pos.IsFinished(); }
    void Render();
    std::string Header;
    std::string Body;
    float Time = 0.f;
    bool ToBeRemoved = false;
    PD::Tween<PD::fvec2> Pos;
    int MyIdx = 0;
    PD::Li::Drawlist Rendered;
    Settings* pSettings = nullptr;
  };

  MsgHandler(PD::Li::Font* f = nullptr, Settings s = Settings()) {
    pFont = f;
    pSettings = s;
  }
  ~MsgHandler() = default;
  PD_SHARED(MsgHandler)

  void Push(const std::string& head, const std::string& Body);
  PD::Li::Drawlist& GetDrawList();
  void Update(float d);

  PD::Li::Font* pFont = nullptr;
  std::vector<std::unique_ptr<Elem>> pElems;
  Settings pSettings;
  PD::Li::Drawlist pDrawData;
};
}  // namespace D7