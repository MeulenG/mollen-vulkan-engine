#ifndef MVE_ENGINE_STATE_H
#define MVE_ENGINE_STATE_H

#include "window.h"
#include "device.h"
#include "renderer.h"
#include "offscreen_pass.h"
#include "imgui_context.h"

#include "../scene/scene.h"
#include "../scene/terrain_streamer.h"
#include "../scene/player_controller.h"
#include "../scene/light_cycle.h"
#include "../scene/components/camera_component.h"
#include "../resources/asset_manager.h"
#include "../resources/dbc_registry.h"
#include "../resources/icon_cache.h"
#include "../db/db_connection.h"
#include "../systems/render_system.h"
#include "../systems/animation_system.h"
#include "../systems/editor_ui_system.h"
#include "../systems/dbc_browser_system.h"
#include "../systems/dbc_form_system.h"
#include "../systems/spell_editor_system.h"

#include <chrono>
#include <memory>

namespace mve {

// All long-lived engine state in one heap-allocated blob. main() (the future
// host) allocates this and drives it through EngineInit / EngineFrame /
// EngineShutdown. Consolidating every subsystem here is the prerequisite for
// the host/module split (HOTRELOAD_PLAN.md phase 4): the host owns this memory
// so it survives a DLL reload while the module's code operates on it.
//
// Objects that take constructor arguments / hold vk::raii handles are held by
// unique_ptr and built in dependency order inside EngineInit. Default-
// constructible subsystems are stored by value. Declaration order here is also
// destruction order (reverse): the Device is declared early so it outlives
// every GPU-resource owner, and the Window outlives the Device's surface.
struct EngineState {
    std::unique_ptr<Window>        window;
    std::unique_ptr<Device>        device;
    std::unique_ptr<Renderer>      renderer;
    std::unique_ptr<ImGuiContext>  imgui_ctx;
    std::unique_ptr<OffscreenPass> offscreen;

    Scene           scene;
    LightCycle      light_cycle;
    AnimationSystem animation_system;
    PlayerController player;

    std::unique_ptr<RenderSystem>    render_system;
    std::unique_ptr<AssetManager>    assets;
    std::unique_ptr<EditorUISystem>  editor_ui;
    std::unique_ptr<TerrainStreamer> streamer;

    // DBC editor tooling (browser, form editor, spell editor). DbConnection
    // is default-constructible and connects best-effort in EngineInit;
    // failure leaves the browser in file-only mode.
    DbConnection db;
    std::unique_ptr<DbcRegistry>       dbc_registry;
    std::unique_ptr<DbcBrowserSystem>  dbc_browser;
    std::unique_ptr<DbcFormSystem>     dbc_form;
    std::unique_ptr<IconCache>         icon_cache;
    std::unique_ptr<SpellEditorSystem> spell_editor;

    // Active editor camera component, living in the scene's component pool.
    // Stable across frames (only one camera entity exists).
    CameraComponent* cam = nullptr;

    std::chrono::high_resolution_clock::time_point last_time{};
};

// Build the window/device/systems and load the world. Equivalent to the old
// main() setup block.
void EngineInit(EngineState& s);

// One frame: input, player update, system updates, streaming, and render.
// Does NOT poll OS events or compute dt - that stays in the host loop.
void EngineFrame(EngineState& s, float dt);

// Release GPU-resident scene state while the systems are still alive. Called
// once before the EngineState blob is destroyed.
void EngineShutdown(EngineState& s);

} // namespace mve

#endif // MVE_ENGINE_STATE_H
