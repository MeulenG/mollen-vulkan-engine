#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "core/offscreen_pass.h"
#include "core/imgui_context.h"
#include "scene/scene.h"
#include "scene/components/camera_component.h"
#include "scene/components/m2_info_component.h"
#include "resources/asset_manager.h"
#include "systems/render_system.h"
#include "systems/animation_system.h"
#include "systems/editor_ui_system.h"

#include <imgui.h>

#include <chrono>
#include <cstdlib>
#include <iostream>

int main() {
    try {
        mve::Window window{1280, 720, "Mollen Wow Tools"};
        mve::Device device{window};
        mve::Renderer renderer{window, device};
        mve::ImGuiContext imgui_ctx{window, device, renderer.getSwapchainImageFormat()};

        auto offscreen = std::make_unique<mve::OffscreenPass>(
            device, 800, 600,
            renderer.getSwapchainImageFormat(),
            renderer.getDepthFormat());

        // Systems
        mve::Scene scene;
        mve::RenderSystem render_system{device, *offscreen};
        render_system.init();

        mve::AssetManager assets{device};
        assets.setDescriptorResources(
            render_system.descriptorLayout(),
            render_system.descriptorPool(),
            render_system.sceneUBOBuffer());

        mve::AnimationSystem animation_system;
        mve::EditorUISystem editor_ui{window, imgui_ctx, *offscreen};

        // Editor camera
        auto* cam_entity = scene.createEntity("EditorCamera");
        auto* cam = cam_entity->addComponent<mve::CameraComponent>();
        cam->pm_is_active = true;
        cam->pm_camera.setOrbit(8.0f, 0.5f, 0.3f);
        cam->pm_camera.setTarget({0.0f, 0.5f, 0.0f});

        // Load model
        if (auto* bear = assets.loadM2IntoScene("assets/Creature/bear/Bear.M2", scene)) {
            if (auto* info = bear->getComponent<mve::M2InfoComponent>()) {
                float height = info->pm_bbox_max.z - info->pm_bbox_min.z;
                float extent = glm::length(info->pm_bbox_max - info->pm_bbox_min);
                cam->pm_camera.setTarget({0.0f, height * 0.4f, 0.0f});
                cam->pm_camera.setOrbit(extent * 1.8f, 0.5f, 0.2f);
            }
        }

        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.shouldClose()) {
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last_time).count();
            last_time = now;

            // ImGui frame
            imgui_ctx.newFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            // Systems
            editor_ui.update(scene, render_system, dt);
            animation_system.update(scene, dt);
            render_system.updateSceneUBO();
            scene.flushDestroyed();

            // Render
            vk::raii::CommandBuffer* cmd;
            if (!renderer.beginFrame(&cmd)) continue;

            // Find active camera
            mve::Camera* active_cam = nullptr;
            scene.each<mve::CameraComponent>([&](mve::Entity&, mve::CameraComponent& cc) {
                if (cc.pm_is_active) active_cam = &cc.pm_camera;
            });

            if (active_cam) {
                render_system.render(scene, *active_cam, *cmd);
            }

            // ImGui to swapchain
            renderer.beginRendering(*cmd, false);
            imgui_ctx.render(*cmd);
            renderer.endRendering(*cmd);
            renderer.endFrame(*cmd);
        }

        device.device().waitIdle();

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
