#include "engine_state.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>

namespace mve {

namespace {

// Newest write-time across all .spv files in the engine's shader output
// directory. Used by the shader-hot-reload watcher: a change here means
// CMake's compile-shaders step (re)ran and a pipeline rebuild is needed.
std::filesystem::file_time_type MaxSpvMtime() {
    namespace fs = std::filesystem;
    fs::file_time_type best{};
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(MVE_SHADER_DIR, ec)) {
        if (ec) break;
        if (entry.path().extension() == ".spv") {
            auto t = fs::last_write_time(entry.path(), ec);
            if (!ec && t > best) best = t;
        }
    }
    return best;
}

} // namespace

void EngineInit(EngineState& s) {
    s.window    = std::make_unique<Window>(1280, 720, "Mollen Wow Tools");
    s.device    = std::make_unique<Device>(*s.window);
    s.renderer  = std::make_unique<Renderer>(*s.window, *s.device);
    s.imgui_ctx = std::make_unique<ImGuiContext>(
        *s.window, *s.device, s.renderer->GetSwapchainImageFormat());
    s.offscreen = std::make_unique<OffscreenPass>(
        *s.device, 800, 600,
        s.renderer->GetSwapchainImageFormat(),
        s.renderer->GetDepthFormat());

    s.render_system = std::make_unique<RenderSystem>(*s.device, *s.offscreen);
    s.render_system->Init();

    s.assets = std::make_unique<AssetManager>(*s.device);
    s.assets->SetDescriptorResources(
        s.render_system->DescriptorLayout(),
        s.render_system->GetDescriptorPool(),
        s.render_system->SceneUBOBuffer());

    // GroundEffect (detail grass) tables. Non-fatal if missing.
    s.assets->LoadGroundEffectTables();

    // Light DBC tables + day/night cycle wired into the render system.
    s.assets->LoadLightTables();
    s.light_cycle.SetTables(&s.assets->Lights());
    s.render_system->SetLightCycle(&s.light_cycle);

    s.editor_ui = std::make_unique<EditorUISystem>(
        *s.window, *s.imgui_ctx, *s.offscreen, *s.device, *s.assets);

    // Editor camera.
    auto* cam_entity = s.scene.CreateEntity("EditorCamera");
    auto* cam = cam_entity->AddComponent<CameraComponent>();
    cam->pm_is_active = true;
    cam->pm_camera.SetOrbit(8.0f, 0.5f, 0.3f);
    cam->pm_camera.SetTarget({0.0f, 0.5f, 0.0f});
    s.cam = cam;

    s.streamer = std::make_unique<TerrainStreamer>(*s.assets, s.scene);
    if (!s.streamer->LoadWdt("World/Maps/Azeroth/Azeroth.wdt")) {
        std::cerr << "WDT load failed - streamer falls back to "
                     "try-every-tile mode\n";
    }

    // Player spawn on the Northshire road, tile (32, 48). Engine is Z-up:
    // renderX = canonical.Y (west), renderY = canonical.X (north),
    // renderZ = height. Third-person camera anchored to the player eye.
    glm::vec3 player_spawn{-9000.0f, -30.0f, 89.0f};
    glm::vec3 eye_target = player_spawn + glm::vec3{0, 0, 1.7f};
    cam->pm_camera.SetTarget(eye_target);
    cam->pm_camera.SetOrbit(15.0f, 0.0f, 0.35f);
    cam->pm_camera.SetMode(CameraMode::ThirdPerson);

    s.player.SetPosition(player_spawn);

    // Preload the 5x5 tile ring around the camera's tile.
    s.streamer->SetRadius(2);
    s.streamer->SetEvictRadius(3);
    int cam_tx, cam_ty;
    s.streamer->EngineToTile(cam->pm_camera.GetPosition(), cam_tx, cam_ty);
    s.streamer->PreloadAround(cam_tx, cam_ty, 2,
                              s.render_system->TerrainDescriptorLayout());

    // Materialize parked doodad placements into instanced entities.
    s.assets->FlushDoodadInstances(s.scene);

    // Load each MODF WMO placement as a meshed entity; fall back to a
    // colored bbox marker on parse failure so the position stays visible.
    size_t wmo_ok = 0, wmo_fail = 0;
    for (const auto& w : s.assets->PendingWmoPlacements()) {
        if (s.assets->LoadWmoPlacement(
                w, s.scene, s.render_system->WmoDescriptorLayout())) {
            ++wmo_ok;
            continue;
        }
        ++wmo_fail;
        RenderSystem::WmoBboxMarker m{};
        m.pos     = w.engine_pos;
        m.extents = w.bbox_extents;
        uint32_t h = w.unique_id * 2654435761u;
        m.color = glm::vec3{
            ((h >>  0) & 0xff) / 255.0f * 0.7f + 0.3f,
            ((h >>  8) & 0xff) / 255.0f * 0.7f + 0.3f,
            ((h >> 16) & 0xff) / 255.0f * 0.7f + 0.3f,
        };
        s.render_system->AddWmoBbox(m);
        std::fprintf(stderr,
            "  WMO (bbox fallback): %s at engine=(%.0f, %.0f, %.0f)\n",
            w.wow_path.c_str(),
            w.engine_pos.x, w.engine_pos.y, w.engine_pos.z);
    }
    std::fprintf(stderr,
        "WMO placements: %zu loaded, %zu fell back to bbox\n",
        wmo_ok, wmo_fail);
    s.assets->ClearPendingWmoPlacements();

    // Seed the shader-watcher baseline so the very first frame doesn't see a
    // bogus delta (current mtime > default-constructed) and rebuild pipelines
    // for nothing.
    s.last_spv_mtime = MaxSpvMtime();

    s.last_time = std::chrono::high_resolution_clock::now();
}

void EngineFrame(EngineState& s) {
    glfwPollEvents();

    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - s.last_time).count();
    s.last_time = now;

    // Shader hot-reload: any .spv timestamp moving past the baseline means
    // CMake's compile-shaders step just ran; rebuild the affected pipelines
    // in-place. Baseline lives in EngineState so it survives C++ reloads too.
    {
        auto m = MaxSpvMtime();
        if (m > s.last_spv_mtime) {
            s.last_spv_mtime = m;
            s.render_system->ReloadPipelines();
            std::fprintf(stderr, "[shader] pipelines reloaded\n");
        }
    }

    CameraComponent* cam = s.cam;

    s.imgui_ctx->NewFrame();

    // Advance the day/night cycle (no-op while paused).
    s.light_cycle.Tick(dt);

    // Noclip toggle (N): ThirdPerson <-> FlyFirstPerson. Re-anchor to the
    // player when returning to ThirdPerson.
    if (ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        if (cam->pm_camera.Mode() == CameraMode::ThirdPerson) {
            cam->pm_camera.SetMode(CameraMode::FlyFirstPerson);
        } else if (cam->pm_camera.Mode() == CameraMode::FlyFirstPerson) {
            cam->pm_camera.SetMode(CameraMode::ThirdPerson);
            cam->pm_camera.SetTarget(s.player.GetEyePos());
        }
    }

    // Player marker stays visible in every mode.
    s.render_system->SetPlayerPos(s.player.GetPosition());

    if (cam->pm_camera.Mode() == CameraMode::ThirdPerson) {
        bool w = ImGui::IsKeyDown(ImGuiKey_W);
        bool a = ImGui::IsKeyDown(ImGuiKey_A);
        bool sk = ImGui::IsKeyDown(ImGuiKey_S);
        bool d = ImGui::IsKeyDown(ImGuiKey_D);
        bool sprint = ImGui::GetIO().KeyShift;
        s.player.Update(dt, cam->pm_camera, w, a, sk, d, sprint);

        // Snap the player's height (Z) to the terrain.
        glm::vec3 pos = s.player.GetPosition();
        float ground_z;
        if (s.assets->GetGroundY(pos, &ground_z)) {
            pos.z = ground_z;
            s.player.SetPosition(pos);
        }

        cam->pm_camera.SetTarget(s.player.GetEyePos());
        s.render_system->SetPlayerPos(s.player.GetPosition());
    }

    // Systems.
    s.editor_ui->Update(s.scene, *s.render_system, dt);
    s.animation_system.Update(s.scene, dt);
    s.render_system->UpdateSceneUBO();

    // Stream tiles around the active camera.
    Camera* stream_cam = nullptr;
    s.scene.Each<CameraComponent>(
        [&](Entity&, CameraComponent& cc) {
            if (cc.pm_is_active) stream_cam = &cc.pm_camera;
        });
    if (stream_cam) {
        s.streamer->Update(stream_cam->GetPosition(),
                           s.render_system->TerrainDescriptorLayout());
        s.assets->FlushDoodadInstances(s.scene);
    }
    s.scene.FlushDestroyed();

    // Render.
    vk::raii::CommandBuffer* cmd;
    if (!s.renderer->BeginFrame(&cmd)) return;

    Camera* active_cam = nullptr;
    s.scene.Each<CameraComponent>(
        [&](Entity&, CameraComponent& cc) {
            if (cc.pm_is_active) active_cam = &cc.pm_camera;
        });

    if (active_cam) {
        s.render_system->Render(s.scene, *active_cam, *cmd);
    }

    s.renderer->BeginRendering(*cmd, false);
    s.imgui_ctx->Render(*cmd);
    s.renderer->EndRendering(*cmd);
    s.renderer->EndFrame(*cmd);
}

void EngineShutdown(EngineState& s) {
    // Wait for in-flight command buffers, then destroy entity GPU resources
    // (descriptor sets etc.) while the RenderSystem + its pool are still alive.
    s.device->GetDevice().waitIdle();
    s.scene.Clear();
}

} // namespace mve
