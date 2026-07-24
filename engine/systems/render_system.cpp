#include "render_system.h"
#include "../scene/components/transform_component.h"
#include "../scene/components/mesh_component.h"
#include "../scene/components/material_component.h"
#include "../scene/components/terrain_component.h"
#include "../scene/terrain_mesh.h"

#include <string>

namespace mve {

RenderSystem::RenderSystem(Device& device, OffscreenPass& offscreen)
    : pm_device{device}, pm_offscreen{offscreen},
      pm_ground_mesh{Mesh::CreateGroundPlane(device, 30.0f)} {

    pm_scene_data.pm_light_dir = glm::normalize(glm::vec3{0.5f, 1.0f, 0.3f});
    pm_scene_data.pm_ambient = 0.15f;
    pm_scene_data.pm_light_color = glm::vec3{1.0f};
    pm_scene_data.pm_light_intensity = 0.85f;
}

void RenderSystem::Init() {
    std::string shader_dir = MVE_SHADER_DIR;

    // Descriptor layout: binding 0 = UBO, binding 1 = texture, binding 2 = bones
    pm_descriptor_layout = DescriptorSetLayoutBuilder{pm_device}
        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
        .Build();

    // Pool sized generously enough for R3 multi-tile streaming + R4
    // doodad spawn. Per-entity sets consume:
    //   M2:      1 UBO + 1 sampler  + 1 storage (bones)
    //   Terrain: 1 UBO + 2 samplers + 1 storage (chunk meta)
    // 1024 entity sets covers a 5x5 tile grid (25 tiles) plus ~900
    // doodads. Memory cost is sub-MB of driver state.
    //
    // FreeDescriptorSet flag lets vk::raii::DescriptorSet release back
    // to the pool when an entity is destroyed (R3 tile eviction).
    // Without it, validation complains and the slot leaks until program
    // exit.
    pm_descriptor_pool = std::make_unique<DescriptorPool>(
        pm_device, 1024,
        std::vector<vk::DescriptorPoolSize>{
            {vk::DescriptorType::eUniformBuffer,        1024},
            {vk::DescriptorType::eCombinedImageSampler, 2048},
            {vk::DescriptorType::eStorageBuffer,        1024},
        },
        vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    // Terrain descriptor layout: UBO + chunk-meta SSBO (fragment) +
    // diffuse 2D-array sampler + alpha 2D-array sampler. Both array
    // samplers are read in the fragment stage only.
    pm_terrain_descriptor_layout = DescriptorSetLayoutBuilder{pm_device}
        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .Build();

    // Scene UBO (shared across all entities)
    pm_scene_ubo = std::make_unique<Buffer>(
        pm_device, sizeof(SceneUBO),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // Model pipeline
    vk::PushConstantRange push_range{vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants)};
    vk::DescriptorSetLayout layouts[] = {*pm_descriptor_layout};
    vk::PipelineLayoutCreateInfo model_layout_info{};
    model_layout_info.setPushConstantRanges(push_range);
    model_layout_info.setSetLayouts(layouts);
    pm_model_pipeline_layout = pm_device.GetDevice().createPipelineLayout(model_layout_info);

    auto model_config = PipelineConfig::DefaultConfig();
    model_config.pipeline_layout = *pm_model_pipeline_layout;
    model_config.color_attachment_format = pm_offscreen.ColorFormat();
    model_config.depth_attachment_format = pm_offscreen.DepthFormat();
    model_config.binding_descriptions = Vertex::GetBindingDescriptions();
    model_config.attribute_descriptions = Vertex::GetAttributeDescriptions();

    pm_model_pipeline = std::make_unique<Pipeline>(
        pm_device, shader_dir + "/basic.vert.spv", shader_dir + "/basic.frag.spv",
        model_config);

    // Background pipeline (fullscreen gradient, no vertex input, no depth)
    pm_bg_pipeline_layout = pm_device.GetDevice().createPipelineLayout({});

    auto bg_config = PipelineConfig::DefaultConfig();
    bg_config.pipeline_layout = *pm_bg_pipeline_layout;
    bg_config.color_attachment_format = pm_offscreen.ColorFormat();
    bg_config.depth_attachment_format = pm_offscreen.DepthFormat();
    bg_config.depth_stencil_info.depthTestEnable = vk::False;
    bg_config.depth_stencil_info.depthWriteEnable = vk::False;
    bg_config.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
    bg_config.binding_descriptions.clear();
    bg_config.attribute_descriptions.clear();

    pm_bg_pipeline = std::make_unique<Pipeline>(
        pm_device, shader_dir + "/background.vert.spv", shader_dir + "/background.frag.spv",
        bg_config);

    // Ground pipeline (alpha blended grid)
    vk::PushConstantRange ground_push{vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants)};
    vk::PipelineLayoutCreateInfo ground_layout_info{};
    ground_layout_info.setPushConstantRanges(ground_push);
    pm_ground_pipeline_layout = pm_device.GetDevice().createPipelineLayout(ground_layout_info);

    auto ground_config = PipelineConfig::DefaultConfig();
    ground_config.pipeline_layout = *pm_ground_pipeline_layout;
    ground_config.color_attachment_format = pm_offscreen.ColorFormat();
    ground_config.depth_attachment_format = pm_offscreen.DepthFormat();
    // Ground shader only consumes position - declare just that to avoid
    // "Vertex attribute not consumed" validation warnings.
    ground_config.binding_descriptions = Vertex::GetBindingDescriptions();
    ground_config.attribute_descriptions = {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
    };
    ground_config.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
    ground_config.color_blend_attachment.blendEnable = vk::True;
    ground_config.color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    ground_config.color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    ground_config.color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
    ground_config.color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    ground_config.color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    ground_config.color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

    pm_ground_pipeline = std::make_unique<Pipeline>(
        pm_device, shader_dir + "/ground.vert.spv", shader_dir + "/ground.frag.spv",
        ground_config);

    // Terrain pipeline. Shares the model pipeline's push-constant layout
    // (mvp + model) since the per-fragment splat work happens off the
    // SSBO + array textures bound via the terrain descriptor set.
    vk::PushConstantRange terrain_push{vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants)};
    vk::DescriptorSetLayout terrain_layouts[] = {*pm_terrain_descriptor_layout};
    vk::PipelineLayoutCreateInfo terrain_layout_info{};
    terrain_layout_info.setPushConstantRanges(terrain_push);
    terrain_layout_info.setSetLayouts(terrain_layouts);
    pm_terrain_pipeline_layout = pm_device.GetDevice().createPipelineLayout(terrain_layout_info);

    auto terrain_config = PipelineConfig::DefaultConfig();
    terrain_config.pipeline_layout         = *pm_terrain_pipeline_layout;
    terrain_config.color_attachment_format = pm_offscreen.ColorFormat();
    terrain_config.depth_attachment_format = pm_offscreen.DepthFormat();
    terrain_config.binding_descriptions    = TerrainVertex::GetBindingDescriptions();
    terrain_config.attribute_descriptions  = TerrainVertex::GetAttributeDescriptions();

    pm_terrain_pipeline = std::make_unique<Pipeline>(
        pm_device, shader_dir + "/terrain.vert.spv", shader_dir + "/terrain.frag.spv",
        terrain_config);
}

void RenderSystem::UpdateSceneUBO() {
    pm_scene_ubo->Write(&pm_scene_data, sizeof(SceneUBO));
}

void RenderSystem::Render(Scene& scene, const Camera& active_camera,
                           const vk::raii::CommandBuffer& cmd) {
    pm_offscreen.BeginRendering(cmd);

    // Background gradient
    pm_bg_pipeline->Bind(cmd);
    cmd.draw(3, 1, 0, 0);

    // Ground plane
    pm_ground_pipeline->Bind(cmd);
    {
        glm::mat4 ground_model{1.0f};
        PushConstants ground_push{};
        ground_push.pm_model = ground_model;
        ground_push.pm_mvp = active_camera.GetProjectionMatrix() * active_camera.GetViewMatrix() * ground_model;
        cmd.pushConstants<PushConstants>(
            *pm_ground_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, ground_push);

        pm_ground_mesh.Bind(cmd);
        pm_ground_mesh.Draw(cmd);
    }

    // Terrain entities. Drawn before model entities so the depth buffer
    // has terrain depth written first; model entities (doodads in R4)
    // then test against it. The two pipelines use different descriptor
    // set layouts so we re-bind a fresh set per terrain entity.
    pm_terrain_pipeline->Bind(cmd);
    scene.Each<TransformComponent, MeshComponent, TerrainComponent>(
        [&](Entity&, TransformComponent& transform, MeshComponent& mesh_comp, TerrainComponent& terrain) {
            if (!mesh_comp.pm_visible || !mesh_comp.pm_mesh) return;
            if (!*terrain.pm_descriptor_set) return;

            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                *pm_terrain_pipeline_layout, 0,
                *terrain.pm_descriptor_set, nullptr);

            glm::mat4 model = transform.ModelMatrix();
            PushConstants push{};
            push.pm_model = model;
            push.pm_mvp = active_camera.GetProjectionMatrix() * active_camera.GetViewMatrix() * model;
            cmd.pushConstants<PushConstants>(
                *pm_terrain_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, push);

            mesh_comp.pm_mesh->Bind(cmd);
            mesh_comp.pm_mesh->Draw(cmd);
        });

    // Render all entities with mesh + transform + material (M2 doodads
    // etc.). Skips terrain entities because they lack MaterialComponent.
    pm_model_pipeline->Bind(cmd);

    scene.Each<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity& entity, TransformComponent& transform, MeshComponent& mesh_comp, MaterialComponent& mat) {
            if (!mesh_comp.pm_visible || !mesh_comp.pm_mesh) return;

            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                *pm_model_pipeline_layout, 0,
                *mat.pm_descriptor_set, nullptr);

            glm::mat4 model = transform.ModelMatrix();
            PushConstants push{};
            push.pm_model = model;
            push.pm_mvp = active_camera.GetProjectionMatrix() * active_camera.GetViewMatrix() * model;
            cmd.pushConstants<PushConstants>(
                *pm_model_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, push);

            mesh_comp.pm_mesh->Bind(cmd);
            mesh_comp.pm_mesh->Draw(cmd);
        });

    pm_offscreen.EndRendering(cmd);
}

} // namespace mve
