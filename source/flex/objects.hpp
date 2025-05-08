#pragma once

#include <flex/container.hpp>

/** Header Containing Simple Objects */

namespace PD {
namespace Flex {
class Rect : public Container {
 public:
  Rect() {}
  Rect(fvec2 p, fvec2 s, u32 clr) {
    pPos = p;
    pSize = s;
    pColor = clr;
  }
  ~Rect() = default;
  PD_SMART_CTOR(Rect)

  Rect& SetPos(fvec2 p) {
    pPos = p;
    return *this;
  }
  Rect& SetSize(fvec2 s) {
    pSize = s;
    return *this;
  }
  Rect& SetColor(u32 c) {
    pColor = c;
    return *this;
  }

  fvec2 GetPos() const { return pPos; }
  fvec2 GetSize() const { return pSize; }
  u32 GetColor() const { return pColor; }

  void Draw(PD::LI::DrawList::Ref l) const override;

  fvec2 pPos;
  fvec2 pSize;
  u32 pColor;
};

class Triangle : public Container {
 public:
  Triangle() {}
  ~Triangle() = default;
  PD_SMART_CTOR(Triangle)

  Triangle& SetPosA(fvec2 p) {
    pPosA = p;
    return *this;
  }
  Triangle& SetPosB(fvec2 s) {
    pPosB = s;
    return *this;
  }
  Triangle& SetPosC(fvec2 s) {
    pPosC = s;
    return *this;
  }
  Triangle& SetColor(u32 c) {
    pColor = c;
    return *this;
  }

  fvec2 GetPosA() const { return pPosA; }
  fvec2 GetPosB() const { return pPosB; }
  fvec2 GetPosC() const { return pPosC; }
  u32 GetColor() const { return pColor; }

  void Draw(PD::LI::DrawList::Ref l) const override;

  fvec2 pPosA;
  fvec2 pPosB;
  fvec2 pPosC;
  u32 pColor;
};

class Button : public Container {
 public:
  Button() {}
  ~Button() {}
  PD_SMART_CTOR(Button)

  void Draw(PD::LI::DrawList::Ref l) const override;

  Button& OnPress(std::function<void()> f) {
    pOnPress = f;
    return *this;
  }

  fvec2 pPos;
  std::string pName;
  std::function<void()> pOnPress;
};

class Text : public Container {
 public:
  Text() {}
  ~Text() = default;
  PD_SMART_CTOR(Text)

  Text& SetPos(fvec2 p) {
    pPos = p;
    return *this;
  }
  Text& SetBoxSize(fvec2 s) {
    pSize = s;
    return *this;
  }
  Text& SetColor(u32 c) {
    pColor = c;
    return *this;
  }
  Text& SetText(const std::string& text) {
    pText = text;
    return *this;
  }

  fvec2 GetPos() const { return pPos; }
  fvec2 GetBoxSize() const { return pSize; }
  u32 GetColor() const { return pColor; }
  const std::string& GetText() const { return pText; }

  void Draw(PD::LI::DrawList::Ref l) const override;

  std::string pText;
  fvec2 pPos;
  fvec2 pSize;
  u32 pColor;
};

class Image : public Container {
 public:
  Image() {}
  ~Image() = default;
  PD_SMART_CTOR(Image)

  Image& SetPos(fvec2 p) {
    pPos = p;
    return *this;
  }
  Image& SetSize(fvec2 s) {
    pSize = s;
    return *this;
  }
  Image& SetColor(u32 c) {
    pColor = c;
    return *this;
  }
  Image& SetImage(PD::LI::Texture::Ref t) {
    pTex = t;
    return *this;
  }
  fvec2 GetPos() const { return pPos; }
  fvec2 GetSize() const { return pSize; }
  u32 GetColor() const { return pColor; }
  PD::LI::Texture::Ref GetImage() const { return pTex; }

  void Draw(PD::LI::DrawList::Ref l) const override;

  PD::LI::Texture::Ref pTex;
  fvec2 pPos;
  fvec2 pSize = fvec2(-1.f, -1.f);
  u32 pColor = 0xffffffff;
};
}  // namespace Flex
}  // namespace PD