#include <flex/objects.hpp>

namespace PD {
namespace Flex {
void Rect::Draw(PD::Li::Drawlist& l) const {
  l.UnbindTexture();
  l.DrawRectFilled(pPos, pSize, pColor);
}
void Triangle::Draw(PD::Li::Drawlist& l) const {
  l.UnbindTexture();
  l.DrawTriangleFilled(pPosA, pPosB, pPosC, pColor);
}
void Text::Draw(PD::Li::Drawlist& l) const {
  // l.pLayer++;
  l.DrawText(pPos, pText.c_str(), pColor);
  // l.pLayer--;
}
void Image::Draw(PD::Li::Drawlist& l) const {
  ivec2 size = pTex.GetSize();
  if (pSize != fvec2(-1.f, -1.f)) {
    size = ivec2(pSize.x, pSize.y);
  }
  // l.pLayer++;
  l.BindTexture(pTex);
  l.DrawRectFilled(pPos, fvec2(size.x, size.y), pColor);
  // l.pLayer--;
}
void Button::Draw(PD::Li::Drawlist& l) const {
  if (!gTheme) {
    return;
  }
  l.DrawRectFilled(pPos, 40, gTheme->Button);
  // l.pLayer++;
  l.DrawText(pPos + 2, pName.c_str(), gTheme->Text);
  // l.pLayer--;
}
}  // namespace Flex
}  // namespace PD