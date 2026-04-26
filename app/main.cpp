#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "core/offscreen_pass.h"
#include "core/imgui_context.h"
#include "scene/scene.h"
#include "scene/components/camera_component.h"
#include "scene/components/m2_info_component.h"
#include "resources/asset_manager.h"
#include "resources/dbc_registry.h"
#include "db/db_connection.h"
#include "systems/render_system.h"
#include "systems/animation_system.h"
#include "systems/editor_ui_system.h"
#include "systems/dbc_browser_system.h"

#include <imgui.h>

#include <chrono>
#include <cstdlib>
#include <iostream>

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};
        mve::Device device{window};
        mve::Renderer renderer{window, device};
        mve::ImGuiContext imgui_ctx{window, device, renderer.GetSwapchainImageFormat()};

        auto offscreen = std::make_unique<mve::OffscreenPass>(
            device, 800, 600,
            renderer.GetSwapchainImageFormat(),
            renderer.GetDepthFormat());

        // Systems
        // Declaration order matters: render_system owns the descriptor pool,
        // scene owns entities whose MaterialComponents allocate descriptor
        // sets from that pool. C++ destroys in reverse, so scene must be
        // declared *after* render_system to die first.
        mve::RenderSystem render_system{device, *offscreen};
        render_system.Init();

        mve::Scene scene;

        mve::AssetManager assets{device};
        assets.SetDescriptorResources(
            render_system.DescriptorLayout(),
            render_system.GetDescriptorPool(),
            render_system.SceneUBOBuffer());

        mve::AnimationSystem animation_system;
        mve::EditorUISystem editor_ui{window, imgui_ctx, *offscreen};

        mve::DbcRegistry dbc_registry{"assets/dbc"};

        // Best-effort connect — failure leaves the browser in file-only mode.
        mve::DbConnection db;
        if (!db.Connect("db_config.toml")) {
            std::cerr << "DB connect: " << db.LastError() << "\n";
        }

        mve::DbcBrowserSystem dbc_browser{dbc_registry, db};

        // Editor camera
        auto* cam_entity = scene.CreateEntity("EditorCamera");
        auto* cam = cam_entity->AddComponent<mve::CameraComponent>();
        cam->pm_is_active = true;
        cam->pm_camera.SetOrbit(8.0f, 0.5f, 0.3f);
        cam->pm_camera.SetTarget({0.0f, 0.5f, 0.0f});

        // Load model
        if (auto* bear = assets.LoadM2IntoScene("assets/Creature/bear/Bear.M2", scene)) {
            if (auto* info = bear->GetComponent<mve::M2InfoComponent>()) {
                float height = info->pm_bbox_max.z - info->pm_bbox_min.z;
                float extent = glm::length(info->pm_bbox_max - info->pm_bbox_min);
                cam->pm_camera.SetTarget({0.0f, height * 0.4f, 0.0f});
                cam->pm_camera.SetOrbit(extent * 1.8f, 0.5f, 0.2f);
            }
        }

        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.ShouldClose()) {
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last_time).count();
            last_time = now;

            // ImGui frame
            imgui_ctx.NewFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            // Systems
            editor_ui.Update(scene, render_system, dt);
            dbc_browser.Update();
            animation_system.Update(scene, dt);
            render_system.UpdateSceneUBO();
            scene.FlushDestroyed();

            // Render
            vk::raii::CommandBuffer* cmd;
            if (!renderer.BeginFrame(&cmd)) continue;

            // Find active camera
            mve::Camera* active_cam = nullptr;
            scene.Each<mve::CameraComponent>([&](mve::Entity&, mve::CameraComponent& cc) {
                if (cc.pm_is_active) active_cam = &cc.pm_camera;
            });

            if (active_cam) {
                render_system.Render(scene, *active_cam, *cmd);
            }

            // ImGui to swapchain
            renderer.BeginRendering(*cmd, false);
            imgui_ctx.Render(*cmd);
            renderer.EndRendering(*cmd);
            renderer.EndFrame(*cmd);
        }

        device.GetDevice().waitIdle();

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
