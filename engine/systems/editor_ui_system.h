#ifndef MVE_EDITOR_UI_SYSTEM_H
#define MVE_EDITOR_UI_SYSTEM_H

#include "../core/window.h"
#include "../core/device.h"
#include "../core/imgui_context.h"
#include "../core/offscreen_pass.h"
#include "../scene/scene.h"
#include "../resources/asset_manager.h"
#include "render_system.h"

#include <imgui.h>

#include <string>
#include <vector>

namespace mve {

class EditorUISystem {
public:
    EditorUISystem(Window& window, ImGuiContext& imgui_ctx,
                   OffscreenPass& offscreen, Device& device,
                   AssetManager& assets);

    void Update(Scene& scene, RenderSystem& render_system, float delta_time);

private:
    // Build the dockspace and (on first launch) lay out the 5-zone
    // default arrangement: Scene on the left, Inspector on the right,
    // Asset Browser at the bottom, Viewport in the center. Re-runs of
    // the splitter are skipped once the dockspace has children so
    // user rearrangements persist via imgui.ini.
    void EnsureDefaultLayout(ImGuiID dockspace_id);

    void DrawMenuBar(Scene& scene);
    void DrawStatusBar(Scene& scene, float delta_time);
    void DrawViewport(Scene& scene, float delta_time);
    void DrawInspector(Scene& scene, RenderSystem& render_system);
    void DrawSceneHierarchy(Scene& scene);
    void DrawAssetBrowser(Scene& scene);
    void DrawStats(Scene& scene, float delta_time);
    void DrawAboutPopup();

    // Cache the recursive scan of MVE_ASSET_DIR. ~13k files takes 1-2
    // seconds to enumerate on a cold filesystem, so we do it lazily on
    // the first DrawAssetBrowser call and again only when the user
    // clicks Refresh.
    void RescanAssets();

    Window& pm_window;
    ImGuiContext& pm_imgui_ctx;
    OffscreenPass& pm_offscreen;
    Device& pm_device;
    AssetManager& pm_assets;

    ImTextureID pm_viewport_tex = ImTextureID_Invalid;

    double pm_last_x = 0.0;
    double pm_last_y = 0.0;
    bool pm_first_mouse = true;

    // Rolling FPS smoother. 60-sample window so the displayed value is
    // stable to ~1 FPS across small jitter, but still tracks larger
    // changes (e.g. when a new tile streams in and stalls).
    static constexpr int kFpsWindow = 60;
    float pm_dt_history[kFpsWindow] = {};
    int   pm_dt_head = 0;
    int   pm_dt_count = 0;

    // First-launch guard for the DockBuilder layout. Once we've laid
    // out the default arrangement we never touch the dockspace again -
    // user rearrangements then persist through imgui.ini exclusively.
    bool pm_layout_built = false;

    // Per-panel visibility, toggled from the View menu. Each Draw* is
    // gated on these so unchecking a menu item makes the corresponding
    // ImGui::Begin call go away entirely.
    bool pm_show_scene         = true;
    bool pm_show_inspector     = true;
    bool pm_show_asset_browser = true;
    bool pm_show_viewport      = true;
    bool pm_show_stats         = false;
    bool pm_show_about         = false;
    bool pm_about_opened       = false; // tracks the OpenPopup call

    // Asset browser cache. pm_asset_paths holds every regular file
    // under MVE_ASSET_DIR, normalized to forward-slashes and stored
    // relative to MVE_ASSET_DIR. pm_assets_scanned is the lazy-init
    // flag; the first DrawAssetBrowser invocation triggers a scan.
    std::vector<std::string> pm_asset_paths;
    bool pm_assets_scanned = false;
    char pm_asset_filter[128] = "";

    // Cached camera state for the status bar. We compute the camera's
    // current tile (32, 48)-style coordinates once per Update so the
    // status bar at the bottom can read them without a second pass.
    int pm_cam_tile_x = -1;
    int pm_cam_tile_y = -1;

    // F12 screenshot counter. Bumped per capture so we keep a history
    // under screenshots/ (in addition to overwriting screenshots/latest.png).
    int pm_screenshot_counter = 0;
};

} // namespace mve

#endif // MVE_EDITOR_UI_SYSTEM_H
