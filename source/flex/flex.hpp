#pragma once
#include <flex/container.hpp>
#include <flex/objects.hpp>
#include <palladium>

/** Another UI Library (worst one ive created) */

namespace PD {
namespace Flex {
class Context {
 public:
  Context(PD::Li::Font::Ref font) {
    pDrawList = PD::Li::DrawList::New();
    pFont = font;
    pDrawList->SetFont(pFont);
  }
  ~Context() {}
  PD_SHARED(Context)

  void DirectAccessDraw(std::function<void(PD::Li::DrawList::Ref)> f) {
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

  Flex::Image& Image(PD::Li::Texture::Ref tex = nullptr) {
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

  PD::Li::DrawList::Ref pDrawList;
  PD::Li::Font::Ref pFont;
  PD::Vec<Container::Ref> pObjects;
};
}  // namespace Flex
}  // namespace PD