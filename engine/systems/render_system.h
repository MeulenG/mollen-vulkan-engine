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

#include <glm/glm.hpp>
#include <memory>

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

    void Render(Scene& scene, const Camera& active_camera,
                const vk::raii::CommandBuffer& cmd);

    void UpdateSceneUBO();

    SceneUBO& SceneData() { return pm_scene_data; }
    const vk::raii::DescriptorSetLayout& DescriptorLayout() const { return pm_descriptor_layout; }
    const vk::raii::DescriptorSetLayout& TerrainDescriptorLayout() const { return pm_terrain_descriptor_layout; }
    mve::DescriptorPool& GetDescriptorPool() { return *pm_descriptor_pool; }
    const Buffer& SceneUBOBuffer() const { return *pm_scene_ubo; }

private:
    Device& pm_device;
    OffscreenPass& pm_offscreen;

    std::unique_ptr<Pipeline> pm_model_pipeline;
    std::unique_ptr<Pipeline> pm_bg_pipeline;
    std::unique_ptr<Pipeline> pm_ground_pipeline;
    std::unique_ptr<Pipeline> pm_terrain_pipeline;

    vk::raii::PipelineLayout pm_model_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_bg_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_ground_pipeline_layout = nullptr;
    vk::raii::PipelineLayout pm_terrain_pipeline_layout = nullptr;

    vk::raii::DescriptorSetLayout pm_descriptor_layout = nullptr;
    vk::raii::DescriptorSetLayout pm_terrain_descriptor_layout = nullptr;
    std::unique_ptr<DescriptorPool> pm_descriptor_pool;
    std::unique_ptr<Buffer> pm_scene_ubo;

    Mesh pm_ground_mesh;

    SceneUBO pm_scene_data{};
};

} // namespace mve

#endif // MVE_RENDER_SYSTEM_H
