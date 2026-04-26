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

    void Update(Scene& scene, RenderSystem& render_system, float delta_time);

private:
    void DrawViewport(Scene& scene, float delta_time);
    void DrawProperties(Scene& scene, RenderSystem& render_system);
    void DrawModelInfo(Scene& scene);
    void DrawSceneHierarchy(Scene& scene);

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
