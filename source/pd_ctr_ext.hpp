#pragma once

#include <pd.hpp>

namespace PD {
namespace Ctr {
struct Context {
  PD::LI::Backend::Ref Gfx;
  PD::Hid::Ref Inp;
  PD::LI::Texture::Ref WhitePixel;
  C3D_RenderTarget* Top;
  C3D_RenderTarget* Bottom;

  PD::Vec<PD::LI::DrawList::Ref> DrawLists[2];
};

Context* CreateContext();
/** Pre alpha feature */
void EnableExceptionScreen();
void DestroyContext(Context* ctx = nullptr);
PD::LI::Texture::Ref GetWhiteTex();
void AddDrawList(PD::LI::DrawList::Ref cmdl, bool bottom);
bool ContextUpdate();
Context& GetContext();
}  // namespace Ctr
}  // namespace PD