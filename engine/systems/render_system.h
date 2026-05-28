#ifndef MVE_RENDER_SYSTEM_H
#define MVE_RENDER_SYSTEM_H

#include "../core/device.h"
#include "../core/offscreen_pass.h"
#include "../resources/pipeline.h"
#include "../resources/buffer.h"
#include "../resources/descriptor.h"
#include "../scene/scene.h"
#include "../scene/camera.h"
#include "../scene/mesh.h"
#include "../scene/light_cycle.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace mve {

// Scene-wide uniforms. Bound at binding 0 of both the M2 and terrain
// descriptor layouts; both fragment shaders read it.
//
// std140 layout: vec3 fields take a full 16-byte slot due to alignas(16);
// the trailing float on each line fits in the same slot. Total = 5 * 16
// = 80 bytes.
//
// Naming follows WoW client's Light.dbc terminology (see wowdev.wiki/
// DB/LightIntBand):
//   pm_direct_color   = LightIntBand row 0 "DirectColor" (sun tint
//                       multiplied by Lambert dot(N, L)). For Elwynn
//                       noon this is a saturated orange (1.00, 0.53,
//                       0.00) which gives the warm midday tint.
//   pm_ambient_color  = LightIntBand row 1 "AmbientColor" (cool blue-
//                       grey at noon, 0.41, 0.51, 0.60). Promoted from
//                       a scalar because the client uses a vec3 here
//                       and a scalar ambient was bleaching the shadow
//                       side of foliage toward grey.
//   pm_fog_color      = LightIntBand row 7 "SkyFogColor" (also used
//                       as the lowest sky band). 0.30, 0.47, 0.56 for
//                       Elwynn noon.
//   pm_fog_start/_end = LightFloatBand rows 0+1. fog_end = row0 / 36
//                       yards; fog_start = fog_end * row1 (the "fog
//                       multiplier"). 125/500 yards for Elwynn.
//   pm_fog_rate       = Power exponent on the linear fog ramp; see
//                       DayNight::CalcFogRate. Effectively bends the
//                       linear (1 - clamp((end-d)/(end-start),0,1))
//                       ramp toward the far end. ~2.875 for Elwynn.
//   pm_light_intensity= Scalar multiplier on direct_color (kept as a
//                       knob even though the client doesn't really
//                       have a separate intensity scalar; useful for
//                       editor lighting overrides).
//
// Math: see Math-Lighting.md for the full derivation. Lighting is
// pure Lambert + colored ambient (no hemispherical / no specular),
// matching the WoW M2 vertex shader. Fog is linear ramp with pow
// shaping, matching `CShaderEffect::SetFogParams` + vertex shader.
struct SceneUBO {
    alignas(16) glm::vec3 pm_light_dir;
    float pm_light_intensity;
    alignas(16) glm::vec3 pm_direct_color;
    float pm_fog_rate;
    alignas(16) glm::vec3 pm_ambient_color;
    float pm_fog_start;
    alignas(16) glm::vec3 pm_fog_color;
    float pm_fog_end;
    alignas(16) glm::vec3 pm_camera_pos;
    float pm_pad;
};

struct PushConstants {
    glm::mat4 pm_mvp;
    glm::mat4 pm_model;
};

class RenderSystem {
public:
    RenderSystem(Device& device, OffscreenPass& offscreen);

    void Init();

    // Rebuild every graphics pipeline from the current .spv files on disk.
    // Used by the shader-hot-reload watcher: when a .spv timestamp changes,
    // the engine waitIdles and calls this, which destroys the old Pipeline
    // objects and recreates them. Layouts/descriptors are NOT touched, so
    // existing descriptor sets and bound buffers remain valid.
    void ReloadPipelines();

    void Render(Scene& scene, const Camera& active_camera,
                const vk::raii::CommandBuffer& cmd);

    void UpdateSceneUBO();

    SceneUBO& SceneData() { return pm_scene_data; }

    // Day/night cycle wiring. SetLightCycle hands the RenderSystem a
    // pointer to the AssetManager-owned cycle; the renderer calls
    // Tick(dt) + Sample(cam, map) each frame to refresh SceneUBO +
    // sky push constants from the interpolated DBC values. nullptr
    // means "no cycle" - the SceneUBO falls back to whatever the
    // constructor seeded (canonical Elwynn noon).
    void SetLightCycle(LightCycle* cycle) { pm_light_cycle = cycle; }
    LightCycle* GetLightCycle() { return pm_light_cycle; }

    // Phase 2B placeholder-player marker. The app calls SetPlayerPos
    // each frame with the player controller's world position; the
    // renderer draws a small colored cube at that position via the
    // ground pipeline. Visibility default = false, so an app that
    // doesn't use a player controller draws nothing extra.
    void SetPlayerPos(const glm::vec3& p) {
        pm_player_pos = p;
        pm_player_visible = true;
    }
    void HidePlayerMarker() { pm_player_visible = false; }

    // Phase 2E stage 1: WMO bounding-box markers. Each entry renders
    // as a colored cube at engine-space `pos`, scaled to (extents.x,
    // extents.y, extents.z) yards. Used as a "the abbey goes here"
    // placeholder until the full WMO geometry loader lands in stage
    // 2. AddWmoBbox is called once per WMO MODF placement at tile
    // load time; the list persists across frames.
    struct WmoBboxMarker {
        glm::vec3 pos{0.0f};
        glm::vec3 extents{1.0f};
        glm::vec3 color{0.6f, 0.5f, 0.4f};
    };
    void AddWmoBbox(const WmoBboxMarker& m) { pm_wmo_bboxes.push_back(m); }
    void ClearWmoBboxes()                   { pm_wmo_bboxes.clear(); }

    const vk::raii::DescriptorSetLayout& DescriptorLayout() const { return pm_descriptor_layout; }
    const vk::raii::DescriptorSetLayout& TerrainDescriptorLayout() const { return pm_terrain_descriptor_layout; }
    mve::DescriptorPool& GetDescriptorPool() { return *pm_descriptor_pool; }
    const Buffer& SceneUBOBuffer() const { return *pm_scene_ubo; }
    const vk::raii::DescriptorSetLayout& WmoDescriptorLayout() const { return pm_wmo_descriptor_layout; }

private:
    // Per-pipeline (re)creation helpers. Each rebuilds just its
    // `pm_*_pipeline` member from the current .spv on disk, leaving the
    // pipeline_layout (and any descriptor-set-layout) members untouched -
    // those are shader-independent and stay across reloads.
    void CreateModelPipelines();
    void CreateBgPipeline();
    void CreateGroundPipeline();
    void CreateTerrainPipeline();
    void CreateWaterPipeline();
    void CreateWmoPipeline();

    Device& pm_device;
    OffscreenPass& pm_offscreen;

    // Per-blend-mode M2 pipeline variants. WoW M2 materials carry a
    // blend_mode field (0..7) that determines opacity, blending, and
    // depth state. Different submeshes within the SAME M2 (e.g. a
    // tree's opaque trunk + alpha-key canopy) need different state.
    //
    // Layout: 4 variants cover the common cases; mode 4-7 (Mod /
    // Mod2x / ModAdd / InvSrcAlphaAdd) fall back to the alpha-blend
    // pipeline for now since they're rare on doodads (mostly used
    // by particle emitters which we don't render yet).
    std::unique_ptr<Pipeline> pm_model_pipeline_opaque;     // blend 0
    std::unique_ptr<Pipeline> pm_model_pipeline_alpha_key;  // blend 1
    std::unique_ptr<Pipeline> pm_model_pipeline_alpha;      // blend 2
    std::unique_ptr<Pipeline> pm_model_pipeline_add;        // blend 3
    std::unique_ptr<Pipeline> pm_bg_pipeline;
    std::unique_ptr<Pipeline> pm_ground_pipeline;
    std::unique_ptr<Pipeline> pm_terrain_pipeline;
    std::unique_ptr<Pipeline> pm_water_pipeline;
    std::unique_ptr<Pipeline> pm_wmo_pipeline;

    // Single layout shared by all 4 model-pipeline variants - they
    // differ only in shader+blend-state, not in descriptor layout.
    vk::raii::PipelineLayout pm_model_pipeline_layout = nullptr;

    // Helper: pick the pipeline for a given M2 blend_mode.
    Pipeline* GetModelPipelineForBlendMode(uint16_t blend_mode) const {
        switch (blend_mode) {
            case 0:  return pm_model_pipeline_opaque.get();
            case 1:  return pm_model_pipeline_alpha_key.get();
            case 2:  return pm_model_pipeline_alpha.get();
            case 3:  return pm_model_pipeline_add.get();
            // Mod / Mod2x / ModAdd / InvSrcAlphaAdd fall back to
            // alpha-blend since their blend states are similar
            // enough for a v1 approximation. A future change can
            // add dedicated pipelines if those modes show up
            // visibly on doodads.
            default: return pm_model_pipeline_alpha.get();
        }
    }
    vk::raii::PipelineLayout pm_bg_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_ground_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_terrain_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_water_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_wmo_pipeline_layout = nullptr;

    vk::raii::DescriptorSetLayout pm_descriptor_layout = nullptr;
    vk::raii::DescriptorSetLayout pm_terrain_descriptor_layout = nullptr;
    vk::raii::DescriptorSetLayout pm_wmo_descriptor_layout = nullptr;
    std::unique_ptr<DescriptorPool> pm_descriptor_pool;
    std::unique_ptr<Buffer> pm_scene_ubo;

    Mesh pm_ground_mesh;

    // Phase 2B placeholder player marker. Small colored cube rendered
    // via the ground pipeline at pm_player_pos each frame (when
    // pm_player_visible == true). Future replacement: a real M2
    // character model with skeleton + animation.
    Mesh pm_player_mesh;
    glm::vec3 pm_player_pos{0.0f};
    bool      pm_player_visible = false;

    // Phase 2E stage 1: WMO bounding-box markers. Each list entry is
    // drawn as one cube via pm_player_mesh (same primitive) translated
    // + scaled to fill the WMO's AABB. Lives until ClearWmoBboxes is
    // called. Future: replaced by real WMO geometry in stage 2.
    std::vector<WmoBboxMarker> pm_wmo_bboxes;

    SceneUBO pm_scene_data{};

    // Day/night cycle pointer. AssetManager owns the LightTables;
    // app/main.cpp constructs the LightCycle and hands it here.
    LightCycle* pm_light_cycle = nullptr;

    // Last-sampled snapshot so the editor UI can display the
    // current band colors without re-sampling.
    LightSnapshot pm_last_snapshot{};
};

} // namespace mve

#endif // MVE_RENDER_SYSTEM_H
