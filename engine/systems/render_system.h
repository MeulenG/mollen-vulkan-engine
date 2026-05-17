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
// std140 layout: vec3 fields take a full 16-byte slot due to alignas(16).
// The trailing float on each line fits in the same slot's pad. Total
// size = 4 * 16 = 64 bytes.
//
// Fog: linear depth-based fog. Pixel color blends toward pm_fog_color
// over the [fog_start, fog_end] camera-distance range. End is set to
// match the far plane (3000) by default so far-clipped geometry fades
// to fog color instead of cutting hard against the sky.
struct SceneUBO {
    alignas(16) glm::vec3 pm_light_dir;
    float pm_ambient;
    alignas(16) glm::vec3 pm_light_color;
    float pm_light_intensity;
    alignas(16) glm::vec3 pm_fog_color;
    float pm_fog_start;
    alignas(16) glm::vec3 pm_camera_pos;
    float pm_fog_end;
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
