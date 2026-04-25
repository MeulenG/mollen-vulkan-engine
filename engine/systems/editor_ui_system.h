#ifndef MVE_EDITOR_UI_SYSTEM_H
#define MVE_EDITOR_UI_SYSTEM_H

#include "../core/window.h"
#include "../core/imgui_context.h"
#include "../core/offscreen_pass.h"
#include "../scene/scene.h"
#include "render_system.h"

#include <imgui.h>

namespace mve {

class EditorUISystem {
public:
    EditorUISystem(Window& window, ImGuiContext& imgui_ctx, OffscreenPass& offscreen);

    void update(Scene& scene, RenderSystem& render_system, float delta_time);

private:
    void drawViewport(Scene& scene, float delta_time);
    void drawProperties(Scene& scene, RenderSystem& render_system);
    void drawModelInfo(Scene& scene);
    void drawSceneHierarchy(Scene& scene);

    Window& pm_window;
    ImGuiContext& pm_imgui_ctx;
    OffscreenPass& pm_offscreen;
    ImTextureID pm_viewport_tex = ImTextureID_Invalid;

    double pm_last_x = 0.0;
    double pm_last_y = 0.0;
    bool pm_first_mouse = true;
};

} // namespace mve

#endif // MVE_EDITOR_UI_SYSTEM_H
