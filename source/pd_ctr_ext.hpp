#pragma once

#include <citro3d.h>

#include <palladium>

namespace PD {
namespace Ctr {
using RCb = std::function<void()>;
struct Context {
  C3D_RenderTarget* Top;
  C3D_RenderTarget* Bottom;
  RCb RenderCbTop = []() {};
  RCb RenderCbBot = []() {};
};

Context* CreateContext();
/** Pre alpha feature */
void EnableExceptionScreen();
void DestroyContext(Context* ctx = nullptr);
void AddRenderCallbackTop(RCb c);
void AddRenderCallbackBottom(RCb c);
bool ContextUpdate();
Context& GetContext();
std::string GetSystemLanguage();
}  // namespace Ctr
}  // namespace PD