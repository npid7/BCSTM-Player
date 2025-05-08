#pragma once
#include <flex/container.hpp>
#include <flex/objects.hpp>
#include <pd.hpp>

/** Another UI Library (worst one ive created) */

namespace PD {
namespace Flex {
class Context {
 public:
  Context(PD::LI::Texture::Ref white, PD::LI::Font::Ref font,
          PD::Hid::Ref inp) {
    pDrawList = PD::LI::DrawList::New(white);
    pFont = font;
    pInp = inp;
    pDrawList->SetFont(pFont);
  }
  ~Context() {}
  PD_SMART_CTOR(Context)

  void DirectAccessDraw(std::function<void(PD::LI::DrawList::Ref)> f) {
    if (f) {
      f(pDrawList);
    }
  }

  Flex::Context& operator<<(Rect::Ref r) {
    pObjects.Add(r);
    return *this;
  }

  Flex::Button& Button(const std::string& name) {
    auto btn = Button::New();
    btn->pName = name;
    pObjects.Add(btn);
    return *(btn.get());
  }

  Flex::Rect& Rect() {
    auto o = Rect::New();
    pObjects.Add(o);
    return *(o.get());
  }

  Flex::Triangle& Triangle() {
    auto o = Triangle::New();
    pObjects.Add(o);
    return *(o.get());
  }

  Flex::Image& Image(PD::LI::Texture::Ref tex = nullptr) {
    auto o = Image::New();
    pObjects.Add(o);
    return o->SetImage(tex);
  }

  Flex::Text& Text(const std::string& text = "") {
    auto o = Text::New();
    pObjects.Add(o);
    return o->SetText(text);
  }

  void Update() {
    for (auto it : pObjects) {
      it->Draw(pDrawList);
    }
    pObjects.Clear();
  }

  PD::LI::DrawList::Ref pDrawList;
  PD::Hid::Ref pInp;
  PD::LI::Font::Ref pFont;
  PD::Vec<Container::Ref> pObjects;
};
}  // namespace Flex
}  // namespace PD