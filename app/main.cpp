#include "core/window.h"
#include "core/device.h"
#include "core/renderer.h"
#include "core/offscreen_pass.h"
#include "core/imgui_context.h"
#include "scene/scene.h"
#include "scene/components/camera_component.h"
#include "scene/components/m2_info_component.h"
#include "scene/components/transform_component.h"
#include "scene/components/mesh_component.h"
#include "scene/components/material_component.h"
#include "scene/components/terrain_component.h"
#include "scene/components/terrain_tile_component.h"
#include "scene/terrain_streamer.h"
#include "resources/asset_manager.h"
#include "resources/buffer.h"
#include "resources/descriptor.h"
#include "resources/image.h"
#include "animation/skeleton.h"
#include "formats/adt_types.h"
#include "systems/render_system.h"
#include "systems/animation_system.h"
#include "systems/editor_ui_system.h"

#include <imgui.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>

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
        mve::Scene scene;
        mve::RenderSystem render_system{device, *offscreen};
        render_system.Init();

        mve::AssetManager assets{device};
        assets.SetDescriptorResources(
            render_system.DescriptorLayout(),
            render_system.GetDescriptorPool(),
            render_system.SceneUBOBuffer());

        mve::AnimationSystem animation_system;
        mve::EditorUISystem editor_ui{window, imgui_ctx, *offscreen};

        // Editor camera
        auto* cam_entity = scene.CreateEntity("EditorCamera");
        auto* cam = cam_entity->AddComponent<mve::CameraComponent>();
        cam->pm_is_active = true;
        cam->pm_camera.SetOrbit(8.0f, 0.5f, 0.3f);
        cam->pm_camera.SetTarget({0.0f, 0.5f, 0.0f});

        // R3: stream multiple ADT tiles around a focus point. The
        // streamer loads tiles in a (2r+1)x(2r+1) ring around the camera,
        // gated by the WDT MAIN bitmap so we never try to open ocean tiles.
        //
        // Initial focus is (32, 48) - Northshire / north Stormwind. We
        // point the camera at that tile's expected engine centroid, then
        // ask the streamer which tile the camera actually ended up in
        // (the orbit position can sit in a neighbor) and preload around
        // that tile - this prevents the first per-frame Update from
        // immediately evicting tiles we just loaded.
        mve::TerrainStreamer streamer{assets, scene};
        if (!streamer.LoadWdt("World/Maps/Azeroth/Azeroth.wdt")) {
            std::cerr << "WDT load failed - streamer falls back to "
                         "try-every-tile mode\n";
        }

        // Hardcoded engine center of tile (32, 48). The per-tile centroid
        // from the file would be slightly more accurate, but the file
        // load happens DURING the preload below - so we use the analytic
        // tile center to bootstrap.
        glm::vec3 center{-8800.0f, 170.0f, -250.0f};
        cam->pm_camera.SetTarget(center);
        // Bigger orbit so the full 3x3 (1600 yards across) fits in view.
        cam->pm_camera.SetOrbit(1500.0f, 0.5f, 0.5f);

        // Preload around wherever the camera actually sits (which may be
        // a neighboring tile because of the orbit offset). Radius 2 (5x5)
        // gives a ~2700-yard view region - enough that the target tile
        // (32, 48) is always inside the loaded set even when the camera
        // sits in a neighbor.
        streamer.SetRadius(2);
        streamer.SetEvictRadius(3);
        int cam_tx, cam_ty;
        streamer.EngineToTile(cam->pm_camera.GetPosition(), cam_tx, cam_ty);
        streamer.PreloadAround(cam_tx, cam_ty, 2,
                                render_system.TerrainDescriptorLayout());

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
            animation_system.Update(scene, dt);
            render_system.UpdateSceneUBO();

            // Stream-load tiles around the camera. Cheap when the camera
            // stays in the same tile; loads ~one new tile per boundary
            // crossing. Runs before FlushDestroyed so evicted tiles
            // disappear in the same frame.
            mve::Camera* stream_cam = nullptr;
            scene.Each<mve::CameraComponent>(
                [&](mve::Entity&, mve::CameraComponent& cc) {
                    if (cc.pm_is_active) stream_cam = &cc.pm_camera;
                });
            if (stream_cam) {
                streamer.Update(stream_cam->GetPosition(),
                                render_system.TerrainDescriptorLayout());
            }
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

        // Shutdown sequence:
        //
        // 1. Wait for any in-flight command buffer that might still be
        //    referencing entity descriptor sets.
        // 2. Destroy every entity NOW, while RenderSystem (and its
        //    DescriptorPool) is still alive. The vk::raii::DescriptorSet
        //    members of TerrainComponent / MaterialComponent free
        //    themselves back to the pool here.
        // 3. The normal stack unwind then tears down render_system
        //    (DescriptorPool dies with nothing left to free), then the
        //    now-empty scene.
        //
        // Without (2), automatic destruction order has render_system
        // dying before scene, and the descriptor-set destructors trip
        // VUID-vkFreeDescriptorSets-descriptorPool-parameter when they
        // try to free against the dead pool.
        device.GetDevice().waitIdle();
        scene.Clear();

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
