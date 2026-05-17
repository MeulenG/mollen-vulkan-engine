#include "editor_ui_system.h"
#include "../scene/components/camera_component.h"
#include "../scene/components/skeleton_component.h"
#include "../scene/components/m2_info_component.h"
#include "../scene/components/terrain_component.h"
#include "../scene/components/terrain_tile_component.h"
#include "../scene/components/mesh_component.h"

namespace mve {

EditorUISystem::EditorUISystem(Window& window, ImGuiContext& imgui_ctx,
                                OffscreenPass& offscreen, Device& device)
    : pm_window{window}, pm_imgui_ctx{imgui_ctx},
      pm_offscreen{offscreen}, pm_device{device} {}

void EditorUISystem::Update(Scene& scene, RenderSystem& render_system, float delta_time) {
    DrawViewport(scene, delta_time);
    DrawProperties(scene, render_system);
    DrawModelInfo(scene);
    DrawSceneHierarchy(scene);
    DrawStats(scene, delta_time);
}

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

            // Scale Pan/Zoom by orbit distance so movement-per-pixel
            // feels constant across zoom levels. At distance 8 (M2 model
            // scale), pan = 8 * 0.001 = 0.008 yards/pixel - close to the
            // old hardcoded 0.005. At distance 1500 (terrain), pan = 1.5
            // yards/pixel - cross the viewport in ~700 px instead of
            // 200,000. Rotate is angular and doesn't need scaling.
            float dist       = active_cam->Distance();
            float pan_speed  = dist * 0.001f;
            float zoom_speed = dist * 0.05f;

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                active_cam->Rotate(float(-dx) * 0.005f, float(dy) * 0.005f);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                active_cam->Pan(float(-dx) * pan_speed, float(dy) * pan_speed);

            float scroll = pm_window.GetScrollDelta();
            if (scroll != 0.0f) active_cam->Zoom(scroll * zoom_speed);
        } else {
            double mx, my;
            pm_window.GetCursorPos(mx, my);
            pm_last_x = mx; pm_last_y = my;
            pm_window.GetScrollDelta();
        }

        if (active_cam) {
            float aspect = viewport_size.x / viewport_size.y;
            // Far plane sized for multi-tile terrain. A single ADT tile is
            // ~533 yards on a side, the camera orbits at ~800 yards, so the
            // back of even a single tile already pushed past the old 1000
            // far plane (tile diagonal / 2 + orbit ~= 1180). R3 will load
            // a 3x3 grid spanning ~1600 yards, so 50000 leaves headroom.
            active_cam->SetPerspective(45.0f, aspect, 0.1f, 50000.0f);
        }
    }
    ImGui::End();
}

void EditorUISystem::DrawProperties(Scene& scene, RenderSystem& render_system) {
    ImGui::Begin("Properties");

    auto& ubo = render_system.SceneData();

    ImGui::SeparatorText("Lighting");
    float light_dir[3] = {ubo.pm_light_dir.x, ubo.pm_light_dir.y, ubo.pm_light_dir.z};
    if (ImGui::SliderFloat3("Light Direction", light_dir, -1.0f, 1.0f)) {
        ubo.pm_light_dir = glm::normalize(glm::vec3{light_dir[0], light_dir[1], light_dir[2]});
    }
    ImGui::SliderFloat("Ambient", &ubo.pm_ambient, 0.0f, 1.0f);
    ImGui::SliderFloat("Intensity", &ubo.pm_light_intensity, 0.0f, 2.0f);

    // Animation controls for selected or first skeleton entity
    ImGui::SeparatorText("Animation");

    scene.Each<SkeletonComponent>([&](Entity& entity, SkeletonComponent& skel) {
        ImGui::PushID(entity.Id());
        ImGui::Checkbox("Animate", &skel.pm_playing);
        ImGui::SliderFloat("Speed", &skel.pm_speed, 0.0f, 3.0f);

        if (!skel.pm_clips.empty()) {
            const char* current_name = skel.pm_clips[skel.pm_current_clip_index]->Name().c_str();
            if (ImGui::BeginCombo("Animation", current_name)) {
                for (int i = 0; i < static_cast<int>(skel.pm_clips.size()); i++) {
                    bool selected = (i == skel.pm_current_clip_index);
                    if (ImGui::Selectable(skel.pm_clips[i]->Name().c_str(), selected)) {
                        skel.pm_current_clip_index = i;
                        if (skel.pm_animator) skel.pm_animator->Play(skel.pm_clips[i]);
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::PopID();
    });

    ImGui::End();
}

void EditorUISystem::DrawModelInfo(Scene& scene) {
    ImGui::Begin("Model");

    bool found = false;
    scene.Each<M2InfoComponent>([&](Entity& entity, M2InfoComponent& info) {
        if (found) return;
        found = true;

        ImGui::Text("Name: %s", info.pm_model_name.c_str());
        ImGui::Text("Vertices: %u", info.pm_vertex_count);
        ImGui::Text("Indices: %u", info.pm_index_count);
        ImGui::Text("Bones: %u", info.pm_bone_count);
        ImGui::Text("Submeshes: %zu", info.pm_submeshes.size());
        ImGui::Text("Textures: %zu", info.pm_texture_paths.size());

        ImGui::SeparatorText("Bounding Box");
        ImGui::Text("Min: (%.1f, %.1f, %.1f)", info.pm_bbox_min.x, info.pm_bbox_min.y, info.pm_bbox_min.z);
        ImGui::Text("Max: (%.1f, %.1f, %.1f)", info.pm_bbox_max.x, info.pm_bbox_max.y, info.pm_bbox_max.z);

        if (ImGui::CollapsingHeader("Texture Paths")) {
            for (size_t i = 0; i < info.pm_texture_paths.size(); i++) {
                const auto& path = info.pm_texture_paths[i];
                ImGui::Text("[%zu] %s", i, path.empty() ? "(replaceable)" : path.c_str());
            }
        }
    });

    if (!found) {
        ImGui::Text("No M2 model loaded.");
    }

    ImGui::End();
}

void EditorUISystem::DrawSceneHierarchy(Scene& scene) {
    ImGui::Begin("Scene");

    for (auto& entity : scene.Entities()) {
        bool selected = (scene.SelectedEntity() == entity->Id());
        if (ImGui::Selectable(entity->Name().c_str(), selected)) {
            scene.SelectEntity(entity->Id());
        }
    }

    ImGui::End();
}

void EditorUISystem::DrawStats(Scene& scene, float delta_time) {
    // Update rolling FPS window. delta_time can hit zero on the first
    // frame; clamp to a microsecond to avoid div-by-zero.
    pm_dt_history[pm_dt_head] = glm::max(delta_time, 1.0e-6f);
    pm_dt_head = (pm_dt_head + 1) % kFpsWindow;
    if (pm_dt_count < kFpsWindow) pm_dt_count++;

    float sum = 0.0f;
    for (int i = 0; i < pm_dt_count; i++) sum += pm_dt_history[i];
    float avg_dt = (pm_dt_count > 0) ? sum / pm_dt_count : 0.0f;
    float avg_fps = (avg_dt > 0.0f) ? 1.0f / avg_dt : 0.0f;

    // Count entities by type. One pass per category - cheap because
    // entity count peaks at ~5000 in R4 and the inner check is a
    // single hash-map lookup per entity.
    int n_terrain = 0;
    int n_doodads = 0;
    int n_total   = static_cast<int>(scene.Entities().size());
    scene.Each<TerrainTileComponent>(
        [&](Entity&, TerrainTileComponent&) { n_terrain++; });
    scene.Each<MeshComponent>(
        [&](Entity& e, MeshComponent&) {
            // A doodad has Mesh + Material (the M2 pipeline) but no
            // TerrainTile. That distinguishes it from terrain entities
            // and from the editor camera (which has no Mesh).
            if (!e.HasComponent<TerrainTileComponent>() &&
                !e.HasComponent<TerrainComponent>()) {
                n_doodads++;
            }
        });

    // Estimate draw calls + triangles. Terrain tiles are 1 draw each at
    // ~65k tris. Doodads are 1 draw each at ~500 tris average (rough -
    // the editor's m2-info panel has exact counts when one is selected).
    int draw_calls = n_terrain + n_doodads;
    int est_tris = n_terrain * 65536 + n_doodads * 500;

    ImGui::Begin("Stats");
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

} // namespace mve
