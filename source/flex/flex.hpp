#pragma once
#include <flex/container.hpp>
#include <flex/objects.hpp>
#include <functional>
#include <palladium>

/** Another UI Library (worst one ive created) */

namespace PD {
namespace Flex {
class Context {
 public:
  Context(PD::Li::Font* font) {
    pFont = font;
    pDrawList.SetFont(pFont);
    pDrawList.SetFontscale(0.525);
  }
  ~Context() {}
  PD_SHARED(Context)

  void DirectAccessDraw(std::function<void(PD::Li::Drawlist*)> f) {
    if (f) {
      f(&pDrawList);
    }
  }

  Flex::Context& operator<<(Rect::Ref r) {
    pObjects.push_back(r);
    return *this;
  }

  Flex::Button& Button(const std::string& name) {
    if (pUsedButton >= pPoolButton.size())
      pPoolButton.push_back(Button::New(nullptr));
    auto& btn = pPoolButton[pUsedButton++];
    btn->pName = name;
    pObjects.push_back(btn);
    return *(btn.get());
  }

  Flex::Rect& Rect() {
    if (pUsedRect >= pPoolRect.size()) pPoolRect.push_back(Rect::New());
    auto& o = pPoolRect[pUsedRect++];
    pObjects.push_back(o);
    return *(o.get());
  }

  Flex::Triangle& Triangle() {
    if (pUsedTriangle >= pPoolTriangle.size())
      pPoolTriangle.push_back(Triangle::New());
    auto& o = pPoolTriangle[pUsedTriangle++];
    pObjects.push_back(o);
    return *(o.get());
  }

  Flex::Image& Image(PD::Li::Texture tex) {
    if (pUsedImage >= pPoolImage.size()) pPoolImage.push_back(Image::New());
    auto& o = pPoolImage[pUsedImage++];
    o->SetImage(tex);
    pObjects.push_back(o);
    return *(o.get());
  }

  Flex::Text& Text(const std::string& text = "") {
    if (pUsedText >= pPoolText.size()) pPoolText.push_back(Text::New());
    auto& o = pPoolText[pUsedText++];
    o->SetText(text);
    pObjects.push_back(o);
    return *(o.get());
  }

  void Update() {
    // pDrawList.Layer(1);
    for (auto it : pObjects) {
      it->Draw(pDrawList);
    }
    pObjects.clear();
    pUsedRect = 0;
    pUsedText = 0;
    pUsedImage = 0;
    pUsedTriangle = 0;
    pUsedButton = 0;
  }

  PD::Li::Drawlist pDrawList;
  PD::Li::Font* pFont;
  std::vector<Container::Ref> pObjects;

  // patchfix this somehow before throwing thsi UI lib completly out of the
  // window
  std::vector<Rect::Ref> pPoolRect;
  int pUsedRect = 0;
  std::vector<Text::Ref> pPoolText;
  int pUsedText = 0;
  std::vector<Image::Ref> pPoolImage;
  int pUsedImage = 0;
  std::vector<Triangle::Ref> pPoolTriangle;
  int pUsedTriangle = 0;
  std::vector<Button::Ref> pPoolButton;
  int pUsedButton = 0;
};
}  // namespace Flex
}  // namespace PD