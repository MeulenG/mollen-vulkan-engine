#include "editor_ui_system.h"
#include "../scene/components/camera_component.h"
#include "../scene/components/skeleton_component.h"
#include "../scene/components/m2_info_component.h"
#include "../scene/components/terrain_component.h"
#include "../scene/components/terrain_tile_component.h"
#include "../scene/components/transform_component.h"
#include "../scene/components/mesh_component.h"
#include "../resources/wmo_debug_tuning.h"

#include <imgui_internal.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>

namespace fs = std::filesystem;

namespace mve {

// Process-wide WMO transform tuning. The render system reads this
// every frame; the inspector slider writes to it. Default values are
// the brute-force starting point - all zeros, as we're searching for
// the correct combination.
WmoDebugTuning g_wmo_debug{};

namespace {

// Convert a string to lowercase in place. Used by the asset-browser
// filter so the user can type any case and still match the cached
// forward-slashed paths (which preserve disk casing).
std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

// Group an asset path by its file extension into one of a few coarse
// buckets the browser tree shows as top-level groups. We deliberately
// do not split by directory yet - that's a deeper tree that would
// dominate the screen on a 13k-file scan.
const char* GroupForPath(const std::string& path) {
    auto dot = path.find_last_of('.');
    if (dot == std::string::npos) return "Other";
    std::string ext = ToLower(path.substr(dot));
    if (ext == ".m2")  return "Models (.m2)";
    if (ext == ".mdx") return "Models (.m2)";
    if (ext == ".blp") return "Textures (.blp)";
    if (ext == ".adt") return "Terrain (.adt)";
    if (ext == ".wdt") return "Terrain (.wdt)";
    if (ext == ".wmo") return "World Models (.wmo)";
    if (ext == ".dbc") return "DBC (.dbc)";
    return "Other";
}

// engine -> tile conversion mirrors TerrainStreamer::EngineToTile.
// Duplicated here so the status bar doesn't need a TerrainStreamer
// reference (the editor UI is rendered every frame and the streamer
// is owned by main, not the scene).
//
// Engine is Z-up: renderX = canonical.Y (west), renderY = canonical.X
// (north). Tile (tile_x, tile_y) covers canonical.X = renderY in
// [(32-tile_x-1)*TILE, (32-tile_x)*TILE] and canonical.Y = renderX
// likewise. tile 32 sits at the world origin.
void CamToTile(const glm::vec3& engine_pos, int& tile_x, int& tile_y) {
    constexpr float kTileSize = 533.3333f;
    int tx = static_cast<int>(32.0f - engine_pos.y / kTileSize);
    int ty = static_cast<int>(32.0f - engine_pos.x / kTileSize);
    tile_x = std::clamp(tx, 0, 63);
    tile_y = std::clamp(ty, 0, 63);
}

} // namespace

EditorUISystem::EditorUISystem(Window& window, ImGuiContext& imgui_ctx,
                                OffscreenPass& offscreen, Device& device,
                                AssetManager& assets)
    : pm_window{window}, pm_imgui_ctx{imgui_ctx},
      pm_offscreen{offscreen}, pm_device{device}, pm_assets{assets} {}

void EditorUISystem::Update(Scene& scene, RenderSystem& render_system, float delta_time) {
    // Two screenshot triggers:
    //   1. F12 key (interactive user). Requires the editor window to
    //      have focus.
    //   2. A flag file at "screenshots/take.flag" (autonomous /
    //      out-of-process trigger). Polled each frame; if present, we
    //      take a shot AND delete the flag. This is what enables an
    //      external script (or me, iterating on visuals) to capture
    //      without fighting Win32 focus-stealing-prevention.
    //
    // Both write to screenshots/latest.png (overwrites) and
    // screenshots/shot_NNNN.png (history).
    namespace fs = std::filesystem;
    bool wants_shot = ImGui::IsKeyPressed(ImGuiKey_F12, false);
    if (!wants_shot && fs::exists("screenshots/take.flag")) {
        wants_shot = true;
        std::error_code ec;
        fs::remove("screenshots/take.flag", ec);
    }
    if (wants_shot) {
        fs::create_directories("screenshots");
        pm_offscreen.SaveColorToPng("screenshots/latest.png");
        char numbered[64];
        std::snprintf(numbered, sizeof(numbered),
                      "screenshots/shot_%04d.png", pm_screenshot_counter++);
        pm_offscreen.SaveColorToPng(numbered);
    }

    // Order matters here. The viewport-side menu bar and status bar
    // claim space from the host viewport's work area; if we drew the
    // dockspace first it would cover them. ImGui handles this
    // automatically as long as the side bars come BEFORE
    // DockSpaceOverViewport.
    DrawMenuBar(scene);
    DrawStatusBar(scene, delta_time);

    // Build (or reuse) the dockspace that wraps the host viewport.
    // DockSpaceOverViewport returns the dockspace's ImGuiID which is
    // the parent node every Draw* below will dock into.
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(
        0, ImGui::GetMainViewport());

    EnsureDefaultLayout(dockspace_id);

    if (pm_show_viewport)      DrawViewport(scene, delta_time);
    if (pm_show_scene)         DrawSceneHierarchy(scene);
    if (pm_show_inspector)     DrawInspector(scene, render_system);
    if (pm_show_asset_browser) DrawAssetBrowser(scene);
    if (pm_show_stats)         DrawStats(scene, delta_time);
    if (pm_show_about)         DrawAboutPopup();
}

// ---------------------------------------------------------------------
// Dock layout
// ---------------------------------------------------------------------
//
// ImGui's DockBuilder API lets us preconfigure the dockspace layout
// once on first launch, then hand off control to the user. The split
// strategy below carves the dockspace into 4 nodes:
//
//   +---------+---------------------+---------+
//   |  left   |   center (Viewport) |  right  |
//   |  Scene  |                     | Insp.   |
//   |         +---------------------+         |
//   |         |  bottom (Assets)    |         |
//   +---------+---------------------+---------+
//
// We split left first (Scene), then right (Inspector), then split
// what's left vertically to get center + bottom. Center stays
// unsplit so it can be used by the Viewport child without
// fighting nested splits.
//
// The 'first launch' check looks at whether the dockspace already
// has children. If imgui.ini brought in a saved layout, the node
// graph is already populated and we leave it alone - user
// rearrangements survive across restarts.

void EditorUISystem::EnsureDefaultLayout(ImGuiID dockspace_id) {
    if (pm_layout_built) return;
    pm_layout_built = true;

    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
    if (node && node->IsSplitNode()) {
        // imgui.ini already gave us a split layout - the user
        // rearranged or we built defaults on a previous run. Honor it.
        return;
    }

    // Reset the node graph (no-op if there isn't one yet, defensive
    // against a half-loaded settings file).
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(
        dockspace_id, ImGui::GetMainViewport()->WorkSize);

    // Split off the left strip for the Scene outliner. 0.18 puts it
    // around 230 px on a 1280-wide window - wide enough for moderately
    // nested entity names without crowding the center viewport.
    ImGuiID left_id;
    ImGuiID remainder_after_left;
    left_id = ImGui::DockBuilderSplitNode(
        dockspace_id, ImGuiDir_Left, 0.18f,
        nullptr, &remainder_after_left);

    // Split off the right strip for the Inspector. 0.22 of the
    // post-left remainder is wider than the Scene panel because
    // the Inspector hosts DragFloat3 widgets and texture lists.
    ImGuiID right_id;
    ImGuiID remainder_after_right;
    right_id = ImGui::DockBuilderSplitNode(
        remainder_after_left, ImGuiDir_Right, 0.22f,
        nullptr, &remainder_after_right);

    // Split the remainder vertically so the bottom strip hosts the
    // Asset Browser. 0.25 gives roughly a 180 px bottom panel on a
    // 720-tall window - plenty for a few rows of file entries
    // without eating the viewport.
    ImGuiID bottom_id;
    ImGuiID center_id;
    bottom_id = ImGui::DockBuilderSplitNode(
        remainder_after_right, ImGuiDir_Down, 0.25f,
        nullptr, &center_id);

    // Dock the panel windows into their nodes. The names here MUST
    // match the ImGui::Begin titles below - DockBuilderDockWindow
    // uses the title string as the key.
    ImGui::DockBuilderDockWindow("Scene",         left_id);
    ImGui::DockBuilderDockWindow("Inspector",     right_id);
    ImGui::DockBuilderDockWindow("Asset Browser", bottom_id);
    ImGui::DockBuilderDockWindow("Console",       bottom_id);
    ImGui::DockBuilderDockWindow("Stats",         bottom_id);
    ImGui::DockBuilderDockWindow("Viewport",      center_id);

    ImGui::DockBuilderFinish(dockspace_id);
}

// ---------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------

void EditorUISystem::DrawMenuBar(Scene& scene) {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Reload Tile", nullptr, false, false)) {
            // Placeholder - actual hook lives in main.cpp where the
            // streamer is owned. R4.5 wires the menu; a later pass
            // will route the request through a small callback.
        }
        if (ImGui::MenuItem("Snapshot", nullptr, false, false)) {
            // Placeholder for an offscreen-pass screenshot.
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Alt+F4")) {
            glfwSetWindowShouldClose(pm_window.GetGLFWWindow(), GLFW_TRUE);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
        ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Scene",         nullptr, &pm_show_scene);
        ImGui::MenuItem("Inspector",     nullptr, &pm_show_inspector);
        ImGui::MenuItem("Asset Browser", nullptr, &pm_show_asset_browser);
        ImGui::MenuItem("Viewport",      nullptr, &pm_show_viewport);
        ImGui::Separator();
        ImGui::MenuItem("Stats",         nullptr, &pm_show_stats);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
        if (ImGui::MenuItem("Reset Camera")) {
            // Snap the active camera back to the editor default tile.
            // (32, 48) is Northshire / north Stormwind, the bootstrap
            // target chosen in main.cpp.
            scene.Each<CameraComponent>(
                [](Entity&, CameraComponent& cc) {
                    if (!cc.pm_is_active) return;
                    cc.pm_camera.SetMode(CameraMode::Orbit);
                    cc.pm_camera.SetTarget({-8800.0f, 170.0f, -250.0f});
                    cc.pm_camera.SetOrbit(1500.0f, 0.5f, 0.5f);
                });
        }
        ImGui::Separator();

        // Camera mode toggle. Orbit (default) keeps a target-centered
        // orbital view for inspecting a model or a fixed location.
        // First-person walk (WASD) drops the camera into a player-eye
        // perspective for navigating the scene.
        bool any_fps = false;
        scene.Each<CameraComponent>(
            [&](Entity&, CameraComponent& cc) {
                if (cc.pm_is_active &&
                    cc.pm_camera.Mode() == CameraMode::FlyFirstPerson) {
                    any_fps = true;
                }
            });
        if (ImGui::MenuItem("Camera: Orbit", nullptr, !any_fps)) {
            scene.Each<CameraComponent>(
                [](Entity&, CameraComponent& cc) {
                    if (cc.pm_is_active) cc.pm_camera.SetMode(CameraMode::Orbit);
                });
        }
        if (ImGui::MenuItem("Camera: First-Person (WASD)", nullptr, any_fps)) {
            scene.Each<CameraComponent>(
                [](Entity&, CameraComponent& cc) {
                    if (cc.pm_is_active) cc.pm_camera.SetMode(CameraMode::FlyFirstPerson);
                });
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Build")) {
        // Synchronously rebuild the engine module. The host's DLL watcher
        // (and the shader watcher) then auto-reload the new binaries on the
        // next frame, so the running app picks up code AND shader changes
        // without a restart. The UI freezes during the build (~5-15s on a
        // warm build); an async + status-panel version is a future polish.
        if (ImGui::MenuItem("Rebuild Engine + Shaders")) {
            std::system("cmake --build build --config Debug "
                        "--target mollen-engine");
        }
        if (ImGui::MenuItem("Rebuild Shaders Only")) {
            std::system("cmake --build build --config Debug "
                        "--target compile-shaders");
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window")) {
        ImGui::MenuItem("Scene",         nullptr, &pm_show_scene);
        ImGui::MenuItem("Inspector",     nullptr, &pm_show_inspector);
        ImGui::MenuItem("Asset Browser", nullptr, &pm_show_asset_browser);
        ImGui::MenuItem("Viewport",      nullptr, &pm_show_viewport);
        ImGui::MenuItem("Stats",         nullptr, &pm_show_stats);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About")) {
            pm_show_about = true;
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void EditorUISystem::DrawAboutPopup() {
    // Two-flag dance to keep OpenPopup idempotent across frames:
    //   pm_show_about    - "the user wants the about box visible"
    //   pm_about_opened  - "OpenPopup has fired for this open request"
    // Without the second flag the OK button would fire CloseCurrentPopup
    // and then the next frame we'd re-OpenPopup because pm_show_about
    // hasn't been cleared yet from inside the popup body.
    if (pm_show_about && !pm_about_opened) {
        ImGui::OpenPopup("About Mollen Wow Tools");
        pm_about_opened = true;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});

    if (ImGui::BeginPopupModal("About Mollen Wow Tools", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Mollen Wow Tools");
        ImGui::Text("Vulkan-based WoW WotLK editor.");
        ImGui::Separator();
        ImGui::Text("Branch: feature/render-elwynn-r4-5-ux");
        ImGui::Text("ImGui:  %s", IMGUI_VERSION);
        ImGui::Separator();
        if (ImGui::Button("OK", {120, 0})) {
            ImGui::CloseCurrentPopup();
            pm_show_about   = false;
            pm_about_opened = false;
        }
        ImGui::EndPopup();
    } else if (pm_about_opened) {
        // BeginPopupModal returned false after we'd opened the popup -
        // user pressed Esc or clicked outside. Reset both flags so
        // the next Help -> About re-opens cleanly.
        pm_show_about   = false;
        pm_about_opened = false;
    }
}

// ---------------------------------------------------------------------
// Viewport panel (3D scene draws here via the offscreen-pass texture)
// ---------------------------------------------------------------------

void EditorUISystem::DrawViewport(Scene& scene, float delta_time) {
    ImGui::Begin("Viewport");
    ImVec2 viewport_size = ImGui::GetContentRegionAvail();

    if (viewport_size.x > 0 && viewport_size.y > 0) {
        uint32_t vp_w = static_cast<uint32_t>(viewport_size.x);
        uint32_t vp_h = static_cast<uint32_t>(viewport_size.y);

        if (vp_w != pm_offscreen.Width() || vp_h != pm_offscreen.Height()) {
            // The previous frame's command buffer is still in flight at
            // this point - it has the OLD viewport descriptor set bound
            // and is sampling the OLD OffscreenPass color image. Freeing
            // either before the GPU finishes is a free-while-in-use bug
            // that surfaces as 'Invalid VkDescriptorPool' or random
            // crashes during interactive panel drags.
            //
            // waitIdle here is the heavy hammer; the panel-drag is
            // already an interactive (slow) event so the stall is
            // imperceptible. A per-frame deferred-retirement queue is
            // the optimization for later.
            pm_device.GetDevice().waitIdle();

            // Free the old descriptor set BEFORE re-registering. ImGui's
            // AddTexture allocates from a fixed-size internal pool; every
            // resize that registers a new texture without freeing the old
            // one leaks one slot.
            pm_imgui_ctx.UnregisterTexture(pm_viewport_tex);
            pm_viewport_tex = ImTextureID_Invalid;

            pm_offscreen.Resize(vp_w, vp_h);
            pm_viewport_tex = pm_imgui_ctx.RegisterTexture(
                *pm_offscreen.GetSampler(), *pm_offscreen.ColorImageView());
        }

        if (pm_viewport_tex == ImTextureID_Invalid) {
            pm_viewport_tex = pm_imgui_ctx.RegisterTexture(
                *pm_offscreen.GetSampler(), *pm_offscreen.ColorImageView());
        }

        ImGui::Image(pm_viewport_tex, viewport_size);

        // Camera input - find active camera
        Camera* active_cam = nullptr;
        scene.Each<CameraComponent>([&](Entity&, CameraComponent& cc) {
            if (cc.pm_is_active) active_cam = &cc.pm_camera;
        });

        if (active_cam && ImGui::IsItemHovered()) {
            double mx, my;
            pm_window.GetCursorPos(mx, my);
            if (pm_first_mouse) { pm_last_x = mx; pm_last_y = my; pm_first_mouse = false; }
            double dx = mx - pm_last_x, dy = my - pm_last_y;
            pm_last_x = mx; pm_last_y = my;

            const bool fps = active_cam->Mode() == CameraMode::FlyFirstPerson;

            // Speeds. Orbit mode scales pan/zoom by distance so they
            // feel constant across zoom levels. FPS mode uses fixed
            // yards-per-second since there's no orbit distance to
            // scale from; held-shift multiplies for "sprint".
            float dist       = active_cam->Distance();
            float pan_speed  = dist * 0.001f;
            float zoom_speed = dist * 0.05f;
            // FPS-mode speed knobs. Base = 25 yd/s (a brisk walk).
            //   Shift = sprint, 5x  -> 125 yd/s
            //   Ctrl  = noclip warp, 20x -> 500 yd/s, crosses a tile in
            //          ~1 second; combined with Shift you get 2500 yd/s
            //          which covers the entire 5x5 preload in under a
            //          second (useful for "verify the layout" surveys).
            float fps_walk   = 25.0f * delta_time;   // yards per frame
            if (ImGui::GetIO().KeyShift) fps_walk *= 5.0f;
            if (ImGui::GetIO().KeyCtrl)  fps_walk *= 20.0f;
            float fps_look   = 0.003f;               // radians per pixel

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || fps) {
                // Right-click drag or any FPS-mode mouse motion rotates
                // the view. FPS mode rotates on any mouse motion if RMB
                // is held (so the user can free-look without losing
                // ImGui interaction with other panels).
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    active_cam->Rotate(float(-dx) * fps_look,
                                       float(-dy) * fps_look);
                }
            }
            if (!fps && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                active_cam->Rotate(float(-dx) * 0.005f, float(dy) * 0.005f);
            }
            if (!fps && ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                active_cam->Pan(float(-dx) * pan_speed, float(dy) * pan_speed);
            }

            float scroll = pm_window.GetScrollDelta();
            if (scroll != 0.0f) active_cam->Zoom(scroll * zoom_speed);

            // WASD for FPS mode: forward / left / back / right + Q/E
            // for up/down. Using ImGui's keyboard state means input is
            // routed correctly (no fight with the ImGui input stack).
            if (fps) {
                float f = 0.0f, s = 0.0f, u = 0.0f;
                if (ImGui::IsKeyDown(ImGuiKey_W)) f += 1.0f;
                if (ImGui::IsKeyDown(ImGuiKey_S)) f -= 1.0f;
                if (ImGui::IsKeyDown(ImGuiKey_D)) s += 1.0f;
                if (ImGui::IsKeyDown(ImGuiKey_A)) s -= 1.0f;
                if (ImGui::IsKeyDown(ImGuiKey_E)) u += 1.0f;
                if (ImGui::IsKeyDown(ImGuiKey_Q)) u -= 1.0f;
                if (f != 0.0f || s != 0.0f || u != 0.0f) {
                    active_cam->Move(s * fps_walk, u * fps_walk, f * fps_walk);
                }
            }
        } else {
            double mx, my;
            pm_window.GetCursorPos(mx, my);
            pm_last_x = mx; pm_last_y = my;
            pm_window.GetScrollDelta();
        }

        if (active_cam) {
            float aspect = viewport_size.x / viewport_size.y;
            // Far plane sized for a 5x5 tile preload viewed from orbit
            // 1500. Tile diagonal is ~754 yards, the loaded 5x5 region
            // spans ~2700 yards. Camera-to-far-corner at orbit 1500 is
            // roughly 2500 yards. 3000 covers the visible area without
            // wasting depth precision on tiles that aren't loaded.
            //
            // Combined with distance fog (see scene UBO), the visible
            // far cutoff is hidden by the fog gradient.
            active_cam->SetPerspective(45.0f, aspect, 1.0f, 3000.0f);
        }
    }
    ImGui::End();
}

// ---------------------------------------------------------------------
// Inspector
// ---------------------------------------------------------------------
//
// The Inspector is a single window whose contents depend on what
// (if anything) is selected in the Scene outliner. The dispatch is:
//
//   selection == NULL_ENTITY -> scene-wide controls (lighting)
//   entity has TransformComponent      -> position/rotation/scale
//   entity has TerrainTileComponent    -> tile (x,y), centroid, radius
//   entity has M2InfoComponent         -> name, counts, bbox, textures
//   entity has SkeletonComponent       -> animation clip selector
//
// A given entity often has several of these (e.g. a doodad has
// Transform + M2Info + Skeleton), so each block is drawn additively
// inside collapsible headers rather than as an if/else.

void EditorUISystem::DrawInspector(Scene& scene, RenderSystem& render_system) {
    ImGui::Begin("Inspector");

    Entity* selected = nullptr;
    if (scene.SelectedEntity() != NULL_ENTITY) {
        selected = scene.FindEntity(scene.SelectedEntity());
    }

    if (selected == nullptr) {
        // No selection: fall back to scene-wide settings. This is
        // where we surface the global lighting controls so they're
        // always reachable even when nothing's selected.
        ImGui::TextDisabled("No entity selected.");
        ImGui::Separator();

        auto& ubo = render_system.SceneData();
        ImGui::SeparatorText("Scene Lighting (WoW Light.dbc semantics)");
        float light_dir[3] = {ubo.pm_light_dir.x, ubo.pm_light_dir.y, ubo.pm_light_dir.z};
        if (ImGui::SliderFloat3("Light Direction", light_dir, -1.0f, 1.0f)) {
            ubo.pm_light_dir = glm::normalize(
                glm::vec3{light_dir[0], light_dir[1], light_dir[2]});
        }
        // Promoted from scalar to vec3 (matches LightIntBand row 1
        // AmbientColor). Color picker so the editor can read the
        // cool-blue vs warm-grey ambient tone at a glance.
        ImGui::ColorEdit3("Direct Color (sun)",  &ubo.pm_direct_color.x);
        ImGui::ColorEdit3("Ambient Color",       &ubo.pm_ambient_color.x);
        ImGui::ColorEdit3("Fog Color",           &ubo.pm_fog_color.x);
        ImGui::SliderFloat("Intensity",          &ubo.pm_light_intensity, 0.0f, 2.0f);
        ImGui::SliderFloat("Fog Start",          &ubo.pm_fog_start, 0.0f, 1000.0f);
        ImGui::SliderFloat("Fog End",            &ubo.pm_fog_end,   0.0f, 2000.0f);
        // Fog rate (the pow exponent on the linear fog ramp). 1.0 =
        // pure linear; higher values bend the ramp toward fog_end
        // (most fog accumulates near the far edge). 2.875 is Elwynn's
        // canonical client-computed value.
        ImGui::SliderFloat("Fog Rate",           &ubo.pm_fog_rate,  0.5f, 7.0f);

        // Live tuning for the WMO model-matrix construction. Used to
        // brute-force the correct combination of magic offset + sign
        // flips needed to make Stormwind/Northshire/etc face the right
        // direction. Changes take effect next frame.
        ImGui::SeparatorText("WMO Transform Tuning (debug)");
        ImGui::SliderFloat("WMO Yaw Offset",
                            &g_wmo_debug.yaw_offset_deg, -360.0f, 360.0f);
        if (ImGui::Button("0"))    g_wmo_debug.yaw_offset_deg = 0.0f;
        ImGui::SameLine();
        if (ImGui::Button("+90"))  g_wmo_debug.yaw_offset_deg = 90.0f;
        ImGui::SameLine();
        if (ImGui::Button("-90"))  g_wmo_debug.yaw_offset_deg = -90.0f;
        ImGui::SameLine();
        if (ImGui::Button("+180")) g_wmo_debug.yaw_offset_deg = 180.0f;
        ImGui::Checkbox("Yaw sign flip",   &g_wmo_debug.yaw_sign_flip);
        ImGui::Checkbox("Pitch sign flip", &g_wmo_debug.pitch_sign_flip);
        ImGui::Checkbox("Roll sign flip",  &g_wmo_debug.roll_sign_flip);
        ImGui::Checkbox("Swap pitch/roll axes", &g_wmo_debug.swap_pitch_roll_axes);
        ImGui::Separator();
        ImGui::TextDisabled("Extra mirror (un-mirror the SwapYZ reflection)");
        ImGui::Checkbox("Mirror X (negate engine.X)", &g_wmo_debug.mirror_x);
        ImGui::Checkbox("Mirror Y (negate engine.Y)", &g_wmo_debug.mirror_y);
        ImGui::Checkbox("Mirror Z (negate engine.Z)", &g_wmo_debug.mirror_z);

        ImGui::End();
        return;
    }

    ImGui::Text("Name: %s", selected->Name().c_str());
    ImGui::Text("ID:   %u", selected->Id());

    // Transform - position / rotation (Euler) / scale. We surface
    // rotation as a quaternion (xyzw) because converting back to
    // Euler can introduce gimbal-flip ambiguity when the user pokes
    // a single axis; quaternion edits are correct but less intuitive.
    if (auto* xf = selected->GetComponent<TransformComponent>()) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", &xf->pm_position.x, 0.1f);

            float quat[4] = {xf->pm_rotation.x, xf->pm_rotation.y,
                             xf->pm_rotation.z, xf->pm_rotation.w};
            if (ImGui::DragFloat4("Rotation (quat)", quat, 0.01f, -1.0f, 1.0f)) {
                xf->pm_rotation = glm::normalize(
                    glm::quat{quat[3], quat[0], quat[1], quat[2]});
            }

            ImGui::DragFloat3("Scale", &xf->pm_scale.x, 0.01f, 0.001f, 100.0f);
        }
    }

    // Terrain tile - all fields read-only because mutating them
    // wouldn't actually move the tile (the streamer would re-evict
    // and the mesh GPU data lives elsewhere).
    if (auto* tt = selected->GetComponent<TerrainTileComponent>()) {
        if (ImGui::CollapsingHeader("Terrain Tile", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Tile: (%d, %d)", tt->pm_tile_x, tt->pm_tile_y);
            ImGui::Text("Centroid: (%.1f, %.1f, %.1f)",
                        tt->pm_centroid_engine.x,
                        tt->pm_centroid_engine.y,
                        tt->pm_centroid_engine.z);
            ImGui::Text("Radius: %.1f", tt->pm_radius);
        }
    }

    // M2 info - name + counts + bbox + texture paths. This replaces
    // the old standalone Model panel which only showed the first
    // M2InfoComponent it found regardless of selection.
    if (auto* info = selected->GetComponent<M2InfoComponent>()) {
        if (ImGui::CollapsingHeader("M2 Model", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Name: %s", info->pm_model_name.c_str());
            ImGui::Text("Vertices: %u", info->pm_vertex_count);
            ImGui::Text("Indices:  %u", info->pm_index_count);
            ImGui::Text("Bones:    %u", info->pm_bone_count);
            ImGui::Text("Submeshes: %zu", info->pm_submeshes.size());
            ImGui::Text("Textures:  %zu", info->pm_texture_paths.size());

            ImGui::SeparatorText("Bounding Box");
            ImGui::Text("Min: (%.1f, %.1f, %.1f)",
                        info->pm_bbox_min.x, info->pm_bbox_min.y, info->pm_bbox_min.z);
            ImGui::Text("Max: (%.1f, %.1f, %.1f)",
                        info->pm_bbox_max.x, info->pm_bbox_max.y, info->pm_bbox_max.z);

            if (ImGui::TreeNode("Texture Paths")) {
                for (size_t i = 0; i < info->pm_texture_paths.size(); i++) {
                    const auto& path = info->pm_texture_paths[i];
                    ImGui::Text("[%zu] %s", i,
                                path.empty() ? "(replaceable)" : path.c_str());
                }
                ImGui::TreePop();
            }
        }
    }

    // Skeleton - animation clip dropdown, play toggle, speed slider.
    if (auto* skel = selected->GetComponent<SkeletonComponent>()) {
        if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushID(selected->Id());
            ImGui::Checkbox("Playing", &skel->pm_playing);
            ImGui::SliderFloat("Speed", &skel->pm_speed, 0.0f, 3.0f);

            if (!skel->pm_clips.empty()) {
                const char* current_name =
                    skel->pm_clips[skel->pm_current_clip_index]->Name().c_str();
                if (ImGui::BeginCombo("Clip", current_name)) {
                    for (int i = 0; i < static_cast<int>(skel->pm_clips.size()); i++) {
                        bool selected_clip = (i == skel->pm_current_clip_index);
                        if (ImGui::Selectable(
                                skel->pm_clips[i]->Name().c_str(), selected_clip)) {
                            skel->pm_current_clip_index = i;
                            if (skel->pm_animator) {
                                skel->pm_animator->Play(skel->pm_clips[i]);
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::PopID();
        }
    }

    // Camera - for the editor camera entity, surface its position so
    // the user can see where they are without computing from the
    // orbit parameters mentally.
    if (auto* cam = selected->GetComponent<CameraComponent>()) {
        if (ImGui::CollapsingHeader("Camera")) {
            glm::vec3 pos = cam->pm_camera.GetPosition();
            glm::vec3 tgt = cam->pm_camera.GetTarget();
            ImGui::Text("Active: %s", cam->pm_is_active ? "yes" : "no");
            ImGui::Text("Pos:    (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
            ImGui::Text("Target: (%.1f, %.1f, %.1f)", tgt.x, tgt.y, tgt.z);
            ImGui::Text("Distance: %.1f", cam->pm_camera.Distance());
        }
    }

    ImGui::End();
}

// ---------------------------------------------------------------------
// Scene outliner
// ---------------------------------------------------------------------

void EditorUISystem::DrawSceneHierarchy(Scene& scene) {
    ImGui::Begin("Scene");

    for (auto& entity : scene.Entities()) {
        bool selected = (scene.SelectedEntity() == entity->Id());
        ImGui::PushID(static_cast<int>(entity->Id()));
        if (ImGui::Selectable(entity->Name().c_str(), selected)) {
            scene.SelectEntity(entity->Id());
        }
        ImGui::PopID();
    }

    ImGui::End();
}

// ---------------------------------------------------------------------
// Asset Browser
// ---------------------------------------------------------------------
//
// Scans MVE_ASSET_DIR recursively the first time the panel is drawn
// (or whenever the user clicks Refresh), groups files by extension,
// shows them in collapsing groups. A filter input above the tree
// narrows the visible set by substring match (case-insensitive).
//
// Thumbnails are explicitly out of scope - rendering BLP previews
// offscreen at editor framerate would be a meaningful chunk of work
// that doesn't unblock anything in R4.5.

void EditorUISystem::RescanAssets() {
    pm_asset_paths.clear();

    std::string root = MVE_ASSET_DIR;
    std::error_code ec;
    auto root_path = fs::path(root);
    if (!fs::exists(root_path, ec)) {
        pm_assets_scanned = true;
        return;
    }

    for (auto it = fs::recursive_directory_iterator(
             root_path, fs::directory_options::skip_permission_denied, ec);
         it != fs::recursive_directory_iterator{}; it.increment(ec)) {
        if (ec) {
            ec.clear();
            continue;
        }
        if (!it->is_regular_file(ec)) continue;

        // Store paths relative to the assets root with forward
        // slashes - that's what the user sees in the tree and what
        // we'll pass to LoadM2IntoScene on double-click.
        auto rel = fs::relative(it->path(), root_path, ec);
        if (ec) { ec.clear(); continue; }
        std::string s = rel.generic_string();
        pm_asset_paths.push_back(std::move(s));
    }

    // Alphabetical order so subsequent re-scans are stable and the
    // visible tree doesn't shuffle across Refresh clicks.
    std::sort(pm_asset_paths.begin(), pm_asset_paths.end());
    pm_assets_scanned = true;
}

void EditorUISystem::DrawAssetBrowser(Scene& scene) {
    ImGui::Begin("Asset Browser");

    if (!pm_assets_scanned) {
        // Lazy first-scan. Done inside the Begin so the user sees the
        // panel paint at least once before the multi-second
        // enumeration kicks off.
        RescanAssets();
    }

    if (ImGui::Button("Refresh")) {
        RescanAssets();
    }
    ImGui::SameLine();
    ImGui::Text("(%zu files)", pm_asset_paths.size());

    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextWithHint("##assetfilter", "filter...",
                              pm_asset_filter,
                              IM_ARRAYSIZE(pm_asset_filter));

    std::string filter_lower = ToLower(pm_asset_filter);

    // Bucket the path list. We rebuild this every frame which is
    // cheap (13k pointer-and-substr operations) but means the
    // tree responds instantly to filter typing.
    struct Group {
        std::vector<const std::string*> paths;
    };
    std::map<std::string, Group> groups;

    for (const auto& path : pm_asset_paths) {
        if (!filter_lower.empty()) {
            std::string p_lower = ToLower(path);
            if (p_lower.find(filter_lower) == std::string::npos) continue;
        }
        groups[GroupForPath(path)].paths.push_back(&path);
    }

    ImGui::BeginChild("##assettree", ImVec2(0, 0), ImGuiChildFlags_Borders);

    for (auto& [group_name, group] : groups) {
        // Header label includes the count so the user can see at a
        // glance how many M2 / BLP / etc. files exist (and whether
        // the filter actually narrowed things).
        std::string header = group_name + " (" +
                              std::to_string(group.paths.size()) + ")";

        // Open the .m2 group by default - the most common interaction
        // is double-clicking a doodad to load it into the scene.
        ImGuiTreeNodeFlags flags = 0;
        if (group_name == "Models (.m2)") {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        if (ImGui::CollapsingHeader(header.c_str(), flags)) {
            // Cap the visible list per group to keep ImGui responsive
            // on the BLP set (8000+ entries). The filter is the
            // intended escape hatch for finding a specific file in
            // the long tail.
            constexpr size_t kMaxVisiblePerGroup = 1024;
            size_t shown = 0;
            for (const auto* p : group.paths) {
                if (shown >= kMaxVisiblePerGroup) {
                    ImGui::TextDisabled(
                        "  ... %zu more (use filter to narrow)",
                        group.paths.size() - shown);
                    break;
                }
                ImGui::PushID(static_cast<int>(reinterpret_cast<uintptr_t>(p) & 0x7fffffff));
                // AllowDoubleClick gives us a clean double-click event
                // without fighting Selectable's default click capture.
                // The ItemHovered + MouseDoubleClicked pattern below
                // covers both single-row Selectable double-click and
                // the case where the user moves between rows mid-click.
                ImGui::Selectable(p->c_str(), false,
                                   ImGuiSelectableFlags_AllowDoubleClick);

                if (ImGui::IsItemHovered() &&
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    // .m2 / .mdx -> spawn into scene at origin.
                    // Other extensions don't have a click-load handler
                    // yet; LoadM2IntoScene will refuse anything that
                    // isn't a valid M2 file so the worst case is a
                    // no-op.
                    std::string ext;
                    auto dot = p->find_last_of('.');
                    if (dot != std::string::npos) ext = ToLower(p->substr(dot));
                    if (ext == ".m2" || ext == ".mdx") {
                        std::string abs = std::string(MVE_ASSET_DIR) + "/" + *p;
                        pm_assets.LoadM2IntoScene(abs, scene);
                    }
                }
                ImGui::PopID();
                shown++;
            }
        }
    }

    ImGui::EndChild();

    ImGui::End();
}

// ---------------------------------------------------------------------
// Stats (opt-in panel - toggle from View menu)
// ---------------------------------------------------------------------

void EditorUISystem::DrawStats(Scene& scene, float delta_time) {
    // FPS/frame are pulled from the rolling window already updated
    // by DrawStatusBar earlier this frame; we just re-display them.
    float sum = 0.0f;
    for (int i = 0; i < pm_dt_count; i++) sum += pm_dt_history[i];
    float avg_dt = (pm_dt_count > 0) ? sum / pm_dt_count : 0.0f;
    float avg_fps = (avg_dt > 0.0f) ? 1.0f / avg_dt : 0.0f;

    int n_terrain = 0;
    int n_doodads = 0;
    int n_total   = static_cast<int>(scene.Entities().size());
    scene.Each<TerrainTileComponent>(
        [&](Entity&, TerrainTileComponent&) { n_terrain++; });
    scene.Each<MeshComponent>(
        [&](Entity& e, MeshComponent&) {
            if (!e.HasComponent<TerrainTileComponent>() &&
                !e.HasComponent<TerrainComponent>()) {
                n_doodads++;
            }
        });

    int draw_calls = n_terrain + n_doodads;
    int est_tris = n_terrain * 65536 + n_doodads * 500;

    ImGui::Begin("Stats", &pm_show_stats);
    ImGui::Text("FPS: %.1f", avg_fps);
    ImGui::Text("Frame: %.2f ms", avg_dt * 1000.0f);
    ImGui::Separator();
    ImGui::Text("Entities: %d", n_total);
    ImGui::Text("  Terrain tiles: %d", n_terrain);
    ImGui::Text("  Doodads:       %d", n_doodads);
    ImGui::Separator();
    ImGui::Text("Draw calls:   %d", draw_calls);
    ImGui::Text("Tris (est):   %.1f M", est_tris / 1.0e6f);
    ImGui::Separator();
    ImGui::TextDisabled("Build Release for ~10x perf");
    ImGui::End();
}

// ---------------------------------------------------------------------
// Status bar (always visible, sits below the dockspace)
// ---------------------------------------------------------------------

void EditorUISystem::DrawStatusBar(Scene& scene, float delta_time) {
    // Update the rolling FPS window here so the status bar (which is
    // drawn first) and the optional Stats panel see the same values.
    pm_dt_history[pm_dt_head] = std::max(delta_time, 1.0e-6f);
    pm_dt_head = (pm_dt_head + 1) % kFpsWindow;
    if (pm_dt_count < kFpsWindow) pm_dt_count++;

    float sum = 0.0f;
    for (int i = 0; i < pm_dt_count; i++) sum += pm_dt_history[i];
    float avg_dt = (pm_dt_count > 0) ? sum / pm_dt_count : 0.0f;
    float avg_fps = (avg_dt > 0.0f) ? 1.0f / avg_dt : 0.0f;

    // Capture the camera's current tile for display. Cheap; the
    // active camera lookup is O(entities) which is fine here since
    // we already iterate the scene every frame.
    glm::vec3 cam_pos{0.0f};
    bool has_cam = false;
    scene.Each<CameraComponent>([&](Entity&, CameraComponent& cc) {
        if (cc.pm_is_active) {
            cam_pos = cc.pm_camera.GetPosition();
            has_cam = true;
        }
    });
    if (has_cam) {
        CamToTile(cam_pos, pm_cam_tile_x, pm_cam_tile_y);
    }

    int n_total = static_cast<int>(scene.Entities().size());

    // BeginViewportSideBar carves a strip out of the main viewport's
    // work area. ImGuiDir_Down + a fixed pixel height gives us a
    // standard one-line status bar; the actual menu-bar pass earlier
    // already took its own strip above the dockspace.
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoSavedSettings |
                              ImGuiWindowFlags_MenuBar;

    float bar_height = ImGui::GetFrameHeight();
    if (ImGui::BeginViewportSideBar("##StatusBar",
                                     ImGui::GetMainViewport(),
                                     ImGuiDir_Down, bar_height, flags)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("FPS: %.1f", avg_fps);
            ImGui::Separator();
            ImGui::Text("Frame: %.2f ms", avg_dt * 1000.0f);
            ImGui::Separator();
            ImGui::Text("Entities: %d", n_total);
            ImGui::Separator();
            if (has_cam) {
                ImGui::Text("Tile: (%d, %d)", pm_cam_tile_x, pm_cam_tile_y);
            } else {
                ImGui::Text("Tile: (-, -)");
            }
            ImGui::Separator();
#ifdef NDEBUG
            ImGui::Text("Build: Release");
#else
            ImGui::Text("Build: Debug");
#endif
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}

} // namespace mve
