#include <flex/objects.hpp>
extern Theme gTheme;

namespace PD {
namespace Flex {
void Rect::Draw(PD::Li::DrawList::Ref l) const {
  l->DrawSolid();
  l->DrawRectFilled(pPos, pSize, pColor);
}
void Triangle::Draw(PD::Li::DrawList::Ref l) const {
  l->DrawSolid();
  l->DrawTriangleFilled(pPosA, pPosB, pPosC, pColor);
}
void Text::Draw(PD::Li::DrawList::Ref l) const {
  l->DrawText(pPos, pText, pColor);
}
void Image::Draw(PD::Li::DrawList::Ref l) const {
  ivec2 size = pTex->GetSize();
  if (pSize != fvec2(-1.f, -1.f)) {
    size = ivec2(pSize.x, pSize.y);
  }
  l->DrawTexture(pTex);
  l->DrawRectFilled(pPos, fvec2(size.x, size.y), pColor);
}
void Button::Draw(PD::Li::DrawList::Ref l) const {
  l->DrawRectFilled(pPos, 40, gTheme.Button);
  l->DrawText(pPos + 2, pName, gTheme.Text);
}
}  // namespace Flex
}  // namespace PD