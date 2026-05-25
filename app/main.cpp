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
#include "scene/player_controller.h"
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

        // Load the GroundEffect DBC tables (texture + doodad) once,
        // before any tiles stream in. ADT tile loading calls
        // ScatterGrassForTile() which silently does nothing if the
        // tables didn't load. A missing or unparseable DBC is logged
        // by LoadGroundEffectTables but isn't fatal - the engine just
        // renders without detail grass.
        assets.LoadGroundEffectTables();

        // Load the Light DBC tables (Light + LightParams + LightIntBand
        // + LightFloatBand) and attach a LightCycle to the render
        // system. The cycle interpolates the 18 LightIntBand colors +
        // 6 LightFloatBand floats across a 16-keyframe per-day curve
        // and drives the scene UBO + sky cone push constants every
        // frame. Without it the renderer uses the hardcoded canonical
        // Elwynn noon values seeded by the RenderSystem constructor.
        assets.LoadLightTables();
        mve::LightCycle light_cycle;
        light_cycle.SetTables(&assets.Lights());
        render_system.SetLightCycle(&light_cycle);

        mve::AnimationSystem animation_system;
        mve::EditorUISystem editor_ui{window, imgui_ctx, *offscreen,
                                       device, assets};

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
        //
        // Default camera is ground-level FPS now - the orbit-1500
        // bird's-eye made the editor read as a diorama. Ground level
        // (matches the target reference) shows trees + terrain at
        // proper density. User can switch via Tools menu.
        // Ground-eye close shot at the Northshire road, designed to
        // catch detail-grass blades scattered around the camera and
        // the cobblestone path in the same frame. orbit 8y radius +
        // pitch +0.15 (~9deg downward look) puts the camera at
        // ~91 yards Y (just above the ground at Y=89) so we're
        // standing on the road looking slightly down at our feet.
        // Grass cull radius is 60 yards so foreground blades render.
        glm::vec3 center{-9014.0f, 89.0f, -2.0f};
        cam->pm_camera.SetTarget(center);
        cam->pm_camera.SetOrbit(8.0f, 1.57f, 0.15f);

        // Phase 2B: third-person mode with a player controller. The
        // player spawns at the Northshire road position; the camera
        // orbits the player's head (eye height = player.y + 1.7).
        // WASD moves the player in camera-relative directions; mouse
        // right-drag rotates the orbit yaw/pitch around the player;
        // scroll zooms orbit distance.
        cam->pm_camera.SetMode(mve::CameraMode::ThirdPerson);
        mve::PlayerController player;
        player.SetPosition(center);

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

        // R4.5: tile loading parks MDDF doodad placements into a
        // per-M2-path pending list rather than spawning one entity per
        // placement. Flush them now that the full 5x5 preload is done
        // so each unique M2 becomes one instanced entity carrying all
        // its placements across every tile that referenced it.
        // Without this call the doodads never enter the scene.
        assets.FlushDoodadInstances(scene);

        auto last_time = std::chrono::high_resolution_clock::now();

        while (!window.ShouldClose()) {
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last_time).count();
            last_time = now;

            // ImGui frame
            imgui_ctx.NewFrame();

            // Advance the day/night cycle clock. light_cycle.Tick
            // wraps to a no-op when paused, which is the editor's
            // default state - the user pins a specific time-of-day
            // for stable comparisons and clicks "Real-time" to let
            // it advance.
            light_cycle.Tick(dt);

            // Phase 2B player update. Runs ONLY when the active camera
            // is in ThirdPerson mode - in Orbit / FlyFirstPerson the
            // WASD keys still drive the camera via EditorUISystem.
            //
            // Read WASD + sprint state straight from ImGui, drive the
            // controller (which composes camera-relative direction +
            // walk speed), then anchor the camera's orbit target to
            // the player's eye position.
            if (cam->pm_camera.Mode() == mve::CameraMode::ThirdPerson) {
                bool w = ImGui::IsKeyDown(ImGuiKey_W);
                bool a = ImGui::IsKeyDown(ImGuiKey_A);
                bool s = ImGui::IsKeyDown(ImGuiKey_S);
                bool d = ImGui::IsKeyDown(ImGuiKey_D);
                bool sprint = ImGui::GetIO().KeyShift;
                player.Update(dt, cam->pm_camera, w, a, s, d, sprint);
                cam->pm_camera.SetTarget(player.GetEyePos());
            }

            // Systems. EditorUISystem owns the dockspace + menu/status
            // bars, so the host viewport gets its layout from there.
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
                // New tiles parked doodad placements; materialize them
                // into instanced entities. No-op when no new tiles
                // loaded this frame.
                assets.FlushDoodadInstances(scene);
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
