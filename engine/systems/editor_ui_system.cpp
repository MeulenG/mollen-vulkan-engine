#include "editor_ui_system.h"
#include "../scene/components/camera_component.h"
#include "../scene/components/skeleton_component.h"
#include "../scene/components/m2_info_component.h"

namespace mve {

EditorUISystem::EditorUISystem(Window& window, ImGuiContext& imgui_ctx, OffscreenPass& offscreen)
    : pm_window{window}, pm_imgui_ctx{imgui_ctx}, pm_offscreen{offscreen} {}

void EditorUISystem::Update(Scene& scene, RenderSystem& render_system, float delta_time) {
    DrawViewport(scene, delta_time);
    DrawProperties(scene, render_system);
    DrawModelInfo(scene);
    DrawSceneHierarchy(scene);
}

void EditorUISystem::DrawViewport(Scene& scene, float delta_time) {
    ImGui::Begin("Viewport");
    ImVec2 viewport_size = ImGui::GetContentRegionAvail();

    if (viewport_size.x > 0 && viewport_size.y > 0) {
        uint32_t vp_w = static_cast<uint32_t>(viewport_size.x);
        uint32_t vp_h = static_cast<uint32_t>(viewport_size.y);

        if (vp_w != pm_offscreen.Width() || vp_h != pm_offscreen.Height()) {
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

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                active_cam->Rotate(float(-dx) * 0.005f, float(dy) * 0.005f);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                active_cam->Pan(float(-dx) * 0.005f, float(dy) * 0.005f);

            float scroll = pm_window.GetScrollDelta();
            if (scroll != 0.0f) active_cam->Zoom(scroll * 0.3f);
        } else {
            double mx, my;
            pm_window.GetCursorPos(mx, my);
            pm_last_x = mx; pm_last_y = my;
            pm_window.GetScrollDelta();
        }

        if (active_cam) {
            float aspect = viewport_size.x / viewport_size.y;
            active_cam->SetPerspective(45.0f, aspect, 0.1f, 1000.0f);
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

} // namespace mve
