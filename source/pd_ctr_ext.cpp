#include <li_backend_c3d.hpp>
#include <pd_ctr_ext.hpp>
#include <pd_hid_3ds.hpp>

constexpr u32 DisplayTransferFlags =
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) |
     GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
     GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

bool pExceptionCtx = false;
bool pExceptionCtxA = false;
/** Seems to not work if it happens in another thread */
void HandleException(const std::string& e) {
  pExceptionCtx = true;
  while (!pExceptionCtxA) {
  }
  gfxInitDefault();
  consoleInit(GFX_TOP, nullptr);
  std::cout << "Palladium Exception Handler\n\n";
  std::cout << "Exeption: \n\n" << e << "\n\n";
  std::cout << "Press Start to Exit!" << std::endl;
  while (aptMainLoop()) {
    hidScanInput();
    if (hidKeysDown() & KEY_START) {
      break;
    }
  }
  gfxExit();
  std::abort();
}

void CxxExceptionHandler() {
  std::exception_ptr e_ = std::current_exception();
  if (e_) {
    try {
      std::rethrow_exception(e_);
    } catch (const std::exception& e) {
      HandleException(e.what());
    } catch (...) {
      HandleException("Unknown Exception");
    }
  }
  std::abort();
}

namespace PD {
namespace Ctr {
Context* ActiveContext;

Context* CheckContext() {
  if (ActiveContext == nullptr) {
    throw std::runtime_error(
        "Context was nullptr. Did you forgot to call PD::Ctr::CreateContext?");
  }
  return ActiveContext;
}

void EnableExceptionScreen() { std::set_terminate(CxxExceptionHandler); }

Context* CreateContext() {
  Context* NewCtx = new Context;
  osSetSpeedupEnable(true);
  romfsInit();
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 2);
  NewCtx->Top =
      C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
  NewCtx->Bottom =
      C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
  C3D_RenderTargetSetOutput(NewCtx->Top, GFX_TOP, GFX_LEFT,
                            DisplayTransferFlags);
  C3D_RenderTargetSetOutput(NewCtx->Bottom, GFX_BOTTOM, GFX_LEFT,
                            DisplayTransferFlags);
  NewCtx->Gfx = PD::LI::Backend_C3D::New();
  NewCtx->Inp = PD::Hid3DS::New();
  NewCtx->Gfx->Init();
  std::vector<PD::u8> wp(16 * 16 * 4, 0xff);
  NewCtx->WhitePixel = NewCtx->Gfx->LoadTexture(wp, 16, 16);
  if (ActiveContext == nullptr) {
    ActiveContext = NewCtx;
  }
  return NewCtx;
}

void DestroyContext(Context* ctx) {
  if (ctx == nullptr) {
    ctx = CheckContext();
  }
  ctx->WhitePixel = nullptr;
  ctx->Inp = nullptr;
  ctx->Gfx->Deinit();
  ctx->Gfx = nullptr;
  C3D_Fini();
  gfxExit();
  pExceptionCtxA = true;
}

bool ContextUpdate() {
  auto c = CheckContext();
  c->Inp->Update();
  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  C3D_RenderTargetClear(c->Top, C3D_CLEAR_ALL, 0x00000000, 0);
  C3D_RenderTargetClear(c->Bottom, C3D_CLEAR_ALL, 0x00000000, 0);
  C3D_FrameDrawOn(c->Top);
  c->Gfx->ViewPort = ivec2(400, 240);
  c->Gfx->NewFrame();
  for (auto& it : c->DrawLists[0]) {
    c->Gfx->RenderDrawData(it->pDrawList);
    it->Clear();
  }
  C3D_FrameDrawOn(c->Bottom);
  c->Gfx->ViewPort = ivec2(320, 240);
  for (auto& it : c->DrawLists[1]) {
    c->Gfx->RenderDrawData(it->pDrawList);
    it->Clear();
  }
  C3D_FrameEnd(0);
  c->DrawLists[0].Clear();
  c->DrawLists[1].Clear();
  return aptMainLoop() && !pExceptionCtx;
}

PD::LI::Texture::Ref GetWhiteTex() {
  auto c = CheckContext();
  return c->WhitePixel;
}

void AddDrawList(PD::LI::DrawList::Ref cmdl, bool bottom) {
  auto c = CheckContext();
  c->DrawLists[bottom].Add(cmdl);
}

Context& GetContext() {
  CheckContext();
  return *ActiveContext;
}
}  // namespace Ctr
}  // namespace PD