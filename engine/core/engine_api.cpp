#include "engine_api.h"
#include "engine_state.h"

#include <imgui.h>
#include <cstdio>

// Thin C wrappers over the C++ EngineState lifecycle. These are the only
// symbols the engine DLL exports; everything else stays internal to the
// module so a reload only ever swaps this code, never the host.

MVE_API mve::EngineState* engine_create() {
    return new mve::EngineState();
}

MVE_API void engine_init(mve::EngineState* s) {
    mve::EngineInit(*s);
}

MVE_API void engine_frame(mve::EngineState* s) {
    mve::EngineFrame(*s);
}

MVE_API bool engine_should_close(mve::EngineState* s) {
    return s->window->ShouldClose();
}

MVE_API void engine_shutdown(mve::EngineState* s) {
    mve::EngineShutdown(*s);
}

MVE_API void engine_destroy(mve::EngineState* s) {
    delete s;
}

MVE_API void engine_on_unload(mve::EngineState* s) {
    // Everything ImGui-side dies with this module (the context carries
    // module function pointers); layout survives as ini text in the blob.
    // Wait for the GPU first - the vulkan backend destroys its font
    // texture and pipeline, which in-flight frames may still reference.
    s->imgui_ini = s->imgui_ctx->SaveSettings();
    s->device->GetDevice().waitIdle();
    s->imgui_ctx->ShutdownForReload();
    s->window->UninstallCallbacks();
}

MVE_API void engine_on_reload(mve::EngineState* s) {
    // Re-point everything at this module: engine window callbacks, then a
    // fresh ImGui context + backend pair with the saved layout replayed.
    s->window->InstallCallbacks();
    s->imgui_ctx->ReinitForReload(*s->window, *s->device,
                                  s->renderer->GetSwapchainImageFormat(),
                                  s->imgui_ini);

    // The validation messenger callback also lives in module code.
    s->device->RecreateDebugMessenger();
}
