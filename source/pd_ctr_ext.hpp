#pragma once

#include <palladium>

namespace PD {
namespace Ctr {
struct Context {
  C3D_RenderTarget* Top;
  C3D_RenderTarget* Bottom;

  std::vector<PD::Li::DrawList::Ref> DrawLists[2];
};

Context* CreateContext();
/** Pre alpha feature */
void EnableExceptionScreen();
void DestroyContext(Context* ctx = nullptr);
void AddDrawList(PD::Li::DrawList::Ref cmdl, bool bottom);
bool ContextUpdate();
Context& GetContext();
std::string GetSystemLanguage();
}  // namespace Ctr
}  // namespace PD