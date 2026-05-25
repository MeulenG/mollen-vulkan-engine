#include "render_system.h"
#include "../scene/components/transform_component.h"
#include "../scene/components/mesh_component.h"
#include "../scene/components/material_component.h"
#include "../scene/components/terrain_component.h"
#include "../scene/components/terrain_tile_component.h"
#include "../scene/components/doodad_instance_component.h"
#include "../scene/terrain_mesh.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace {

// 6 frustum planes (left, right, bottom, top, near, far) extracted
// from a column-major view-projection matrix per the Gribb/Hartmann
// 2001 paper. Each plane is (n.x, n.y, n.z, d) where n is the inward
// normal and `dot(n, p) + d >= 0` means p is on or inside the plane.
std::array<glm::vec4, 6> ExtractFrustumPlanes(const glm::mat4& vp) {
    std::array<glm::vec4, 6> planes;
    // Left  = row4 + row1
    planes[0] = glm::vec4{
        vp[0][3] + vp[0][0], vp[1][3] + vp[1][0],
        vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]};
    // Right = row4 - row1
    planes[1] = glm::vec4{
        vp[0][3] - vp[0][0], vp[1][3] - vp[1][0],
        vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]};
    // Bottom = row4 + row2
    planes[2] = glm::vec4{
        vp[0][3] + vp[0][1], vp[1][3] + vp[1][1],
        vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]};
    // Top = row4 - row2
    planes[3] = glm::vec4{
        vp[0][3] - vp[0][1], vp[1][3] - vp[1][1],
        vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]};
    // Near = row4 + row3
    planes[4] = glm::vec4{
        vp[0][3] + vp[0][2], vp[1][3] + vp[1][2],
        vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]};
    // Far = row4 - row3
    planes[5] = glm::vec4{
        vp[0][3] - vp[0][2], vp[1][3] - vp[1][2],
        vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]};
    for (auto& p : planes) {
        float len = glm::length(glm::vec3(p));
        if (len > 0.0f) p /= len;
    }
    return planes;
}

// Sphere vs frustum. Returns false if the sphere is fully outside any
// of the 6 planes; true otherwise (inside or intersecting). The bias
// (radius) is added so a sphere straddling a plane still counts as
// visible.
bool SphereInFrustum(const std::array<glm::vec4, 6>& planes,
                      const glm::vec3& center, float radius) {
    for (const auto& p : planes) {
        float d = glm::dot(glm::vec3(p), center) + p.w;
        if (d < -radius) return false;
    }
    return true;
}

}  // namespace

namespace mve {

RenderSystem::RenderSystem(Device& device, OffscreenPass& offscreen)
    : pm_device{device}, pm_offscreen{offscreen},
      pm_ground_mesh{Mesh::CreateGroundPlane(device, 30.0f)},
      // Phase 2B placeholder player: a 1x1x1 yard cube. Purple so it
      // doesn't visually blend with the canonical green/brown/yellow
      // Elwynn palette. Will be replaced by a real M2 character in
      // Phase 2C once skeleton + animation lands.
      pm_player_mesh{Mesh::CreateCube(device, glm::vec3{0.5f, 0.2f, 0.8f})} {

    // Canonical Elwynn Forest "noon clear" values, pulled from the
    // WoW client's Light.dbc / LightParams.dbc / LightIntBand.dbc /
    // LightFloatBand.dbc tables (WotLK 3.3.5a 12340 binaries). Elwynn
    // has no zone-local light entry on Map 0 Azeroth, so it falls back
    // to the worldwide default Light.dbc ID=1 -> ParamsClear =
    // LightParams.dbc ID=12 -> the LightIntBand/FloatBand rows below.
    //
    // BGRA -> RGB conversion (client stores colors BGRA in the DBC):
    //   byte0 = B, byte1 = G, byte2 = R, byte3 = A
    // Values are normalized to [0..1] by dividing by 255.
    //
    // Previously these were hand-tuned by eyeballing screenshots,
    // which biased everything toward a stormy/overcast outlier
    // weather state. The DBC values are the actual canonical Elwynn
    // identity ("bright, warm afternoon, autumn-tinged amber
    // canopies, olive-yellow grass"). See docs / wowdev.wiki for the
    // full DBC chain.

    // Sun direction. WoW client computes this from DayNight phi/theta
    // tables: at t=0.5 (noon) phi ~= 2.217 rad, theta ~= 3.927 rad.
    // sunPos = (sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi))
    //        ~= (-0.566, -0.566, -0.601)
    // We want lightDir to point AT the surface (so dot(N, lightDir) is
    // negative for lit faces), so the shader uses -lightDir as L. We
    // store sunPos directly; the shaders negate.
    pm_scene_data.pm_light_dir = glm::normalize(glm::vec3{-0.566f, -0.566f, -0.601f});
    pm_scene_data.pm_light_intensity = 1.0f;

    // LightIntBand row 0 "DirectColor" noon = #FF8800 BGRA -> RGB.
    // This is the sun's tint, multiplied by Lambert dot(N, L). A
    // saturated orange might look extreme on paper, but the bulk of
    // the lit color comes from the texture/MCCV combine and this only
    // applies where N actually faces the sun. The result reads "warm
    // afternoon" rather than monochrome.
    pm_scene_data.pm_direct_color = glm::vec3{1.000f, 0.533f, 0.000f};

    // LightIntBand row 1 "AmbientColor" noon = #68829A BGRA -> RGB.
    // Cool blue-grey, applied to every fragment regardless of N. The
    // mathematical pairing of warm sun + cool ambient is the
    // photometric standard for outdoor scenes: it produces the
    // "lit side warm, shadow side cool" color separation that gives
    // depth without needing GI.
    pm_scene_data.pm_ambient_color = glm::vec3{0.408f, 0.510f, 0.604f};

    // LightIntBand row 7 "SkyFogColor" noon = #4D788F BGRA -> RGB.
    // Also drives the lowest sky band and the background gradient
    // (background.frag reads this via push constant).
    pm_scene_data.pm_fog_color = glm::vec3{0.302f, 0.471f, 0.561f};

    // LightFloatBand at noon:
    //   row 0 = FogEnd (DBC value / 36 -> yards) = 18000 / 36 = 500
    //   row 1 = FogMultiplier = 0.25 -> fog_start = 500 * 0.25 = 125
    // The client always derives start as end * multiplier; it never
    // stores start directly.
    pm_scene_data.pm_fog_start = 125.0f;
    pm_scene_data.pm_fog_end   = 500.0f;

    // Fog rate exponent. From DayNight::CalcFogRate:
    //   farClip   = max(500, cameraFar - 200)   // we pick 500
    //   fogRange  = fog_end - fog_start         // 500 - 125 = 375
    //   rate      = (1 - fogRange/farClip) * 5.5 + 1.5
    //             = (1 - 375/500) * 5.5 + 1.5 = 2.875
    // Effectively this bends the linear fog ramp so the bulk of the
    // fog accumulates near fog_end, which matches how distant
    // mountains in WoW go to haze suddenly rather than gradually.
    pm_scene_data.pm_fog_rate = 2.875f;

    pm_scene_data.pm_camera_pos = glm::vec3{0.0f};
    pm_scene_data.pm_pad = 0.0f;
}

void RenderSystem::Init() {
    std::string shader_dir = MVE_SHADER_DIR;

    // Descriptor layout (M2 pipeline):
    //   binding 0  scene UBO        (fragment)
    //   binding 1  diffuse texture  (fragment)
    //   binding 2  bone matrices    (vertex)  - identity for static doodads
    //   binding 3  instance models  (vertex)  - 1 entry for legacy path,
    //                                           N entries for doodad path
    pm_descriptor_layout = DescriptorSetLayoutBuilder{pm_device}
        .AddBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .AddBinding(2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
        .AddBinding(3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
        .Build();

    // Pool sized for R3 multi-tile + R4.5 instanced doodad density.
    // Per-entity sets consume:
    //   M2:      1 UBO + 1 sampler  + 2 storage (bones + instances)
    //   Terrain: 1 UBO + 2 samplers + 1 storage (chunk meta)
    //
    // R4.5 collapses ~4650 doodad placements into ~50 instanced entities
    // (one per unique M2 path). So the M2 set demand drops dramatically
    // and the pool's main consumer is now terrain tiles (25 at radius
    // 2). 2048 gives plenty of headroom even if a future change loosens
    // dedup or adds animated doodads.
    //
    // FreeDescriptorSet flag lets vk::raii::DescriptorSet release back
    // to the pool when an entity is destroyed (R3 tile eviction).
    pm_descriptor_pool = std::make_unique<DescriptorPool>(
        pm_device, 2048,
        std::vector<vk::DescriptorPoolSize>{
            {vk::DescriptorType::eUniformBuffer,        2048},
            {vk::DescriptorType::eCombinedImageSampler, 4096},
            {vk::DescriptorType::eStorageBuffer,        4096},
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

    // M2 pipeline variants: one per blend mode. The Vulkan pipeline
    // state for each mode is set per wowserhq's Wrath-Blending table
    // (reverse-engineered from the WoW client). All variants share:
    //   - pipeline layout
    //   - vertex input bindings (Vertex)
    //   - cullMode = eNone (M2 leaf-plane material flag 0x04
    //     two-sided semantics; trunks lose nothing from running
    //     un-culled because their back faces are occluded by front
    //     faces and fail the depth test anyway)
    //   - depth-test enabled
    // They differ in:
    //   - fragment shader (opaque uses basic_opaque.frag without
    //     discard; the rest use basic.frag with discard)
    //   - depth-write (true for opaque/alpha-key, false for
    //     translucent so back-to-front draws don't occlude each
    //     other)
    //   - color blend state (off for opaque/alpha-key, src-alpha
    //     blend for mode 2, additive for mode 3)
    auto common_config = PipelineConfig::DefaultConfig();
    common_config.pipeline_layout = *pm_model_pipeline_layout;
    common_config.color_attachment_format = pm_offscreen.ColorFormat();
    common_config.depth_attachment_format = pm_offscreen.DepthFormat();
    common_config.binding_descriptions = Vertex::GetBindingDescriptions();
    common_config.attribute_descriptions = Vertex::GetAttributeDescriptions();
    common_config.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;

    // Blend mode 0: Opaque. No alpha test, no blend, depth write on.
    // Used by trunks, rocks, building parts.
    {
        auto cfg = common_config;
        cfg.color_blend_attachment.blendEnable = vk::False;
        cfg.depth_stencil_info.depthWriteEnable = vk::True;
        pm_model_pipeline_opaque = std::make_unique<Pipeline>(
            pm_device,
            shader_dir + "/basic.vert.spv",
            shader_dir + "/basic_opaque.frag.spv",
            cfg);
    }

    // Blend mode 1: AlphaKey. Discard at 128/255 in shader, no
    // blend, depth write on. Used by tree canopies, fences, alpha-
    // cut foliage.
    {
        auto cfg = common_config;
        cfg.color_blend_attachment.blendEnable = vk::False;
        cfg.depth_stencil_info.depthWriteEnable = vk::True;
        pm_model_pipeline_alpha_key = std::make_unique<Pipeline>(
            pm_device,
            shader_dir + "/basic.vert.spv",
            shader_dir + "/basic.frag.spv",
            cfg);
    }

    // Blend mode 2: Alpha. SrcAlpha / OneMinusSrcAlpha. Depth write
    // off (translucents must accumulate). Used by smoke, particles,
    // some glow halos.
    {
        auto cfg = common_config;
        cfg.color_blend_attachment.blendEnable = vk::True;
        cfg.color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        cfg.color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        cfg.color_blend_attachment.colorBlendOp        = vk::BlendOp::eAdd;
        cfg.color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        cfg.color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        cfg.color_blend_attachment.alphaBlendOp        = vk::BlendOp::eAdd;
        cfg.depth_stencil_info.depthWriteEnable = vk::False;
        // We reuse basic.frag here so the shader still does the
        // discard at 128/255. For pure alpha blending that's a
        // slight precision quirk (alpha just above threshold
        // contributes faintly) but it's the WoW client behaviour
        // for mode 2 submeshes that happen to have hard-edged
        // alpha textures, and harmless for smooth-alpha textures.
        pm_model_pipeline_alpha = std::make_unique<Pipeline>(
            pm_device,
            shader_dir + "/basic.vert.spv",
            shader_dir + "/basic.frag.spv",
            cfg);
    }

    // Blend mode 3: Add. SrcAlpha / One. Pure additive accumulation;
    // useful for glow / magic effects / fairy sparkles. Depth write
    // off; depth-test still on so geometry behind opaque occluders
    // doesn't bleed through.
    {
        auto cfg = common_config;
        cfg.color_blend_attachment.blendEnable = vk::True;
        cfg.color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        cfg.color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOne;
        cfg.color_blend_attachment.colorBlendOp        = vk::BlendOp::eAdd;
        cfg.color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        cfg.color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eOne;
        cfg.color_blend_attachment.alphaBlendOp        = vk::BlendOp::eAdd;
        cfg.depth_stencil_info.depthWriteEnable = vk::False;
        pm_model_pipeline_add = std::make_unique<Pipeline>(
            pm_device,
            shader_dir + "/basic.vert.spv",
            shader_dir + "/basic.frag.spv",
            cfg);
    }

    // Background pipeline (fullscreen gradient, no vertex input, no depth).
    // Takes the canonical Elwynn 6-band sky-cone colors as push
    // constants. The pipeline doesn't need a descriptor set (the
    // shader is purely a procedural gradient), so push-constants are
    // cheaper than a UBO binding for the ~80 bytes we need. Layout:
    //   bytes 0..15:  sky_top    (zenith)
    //   bytes 16..31: sky_middle (sky body)
    //   bytes 32..47: sky_band1  (just above horizon)
    //   bytes 48..63: sky_band2  (right at horizon)
    //   bytes 64..79: fog_color  (below horizon - matches scene fog)
    vk::PushConstantRange bg_push_range{
        vk::ShaderStageFlagBits::eFragment, 0, 5 * sizeof(glm::vec4)};
    vk::PipelineLayoutCreateInfo bg_layout_info{};
    bg_layout_info.setPushConstantRanges(bg_push_range);
    pm_bg_pipeline_layout = pm_device.GetDevice().createPipelineLayout(bg_layout_info);

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
    // Ground shader only consumes position — declare just that to avoid
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

    // Stamp the camera position into the UBO so the fragment shaders'
    // fog math has it. Host-coherent + host-visible memory, so the
    // write is visible to the GPU immediately - no flush required.
    glm::vec3 cam_pos = active_camera.GetPosition();
    pm_scene_data.pm_camera_pos = cam_pos;

    // Day/night cycle: if a LightCycle is wired, sample the
    // interpolated Light DBC snapshot for the current time + camera
    // position and copy the relevant fields into SceneUBO. The
    // hardcoded "Elwynn noon" values from the constructor remain as
    // a fallback when no cycle is attached or its tables didn't
    // load. Map ID = 0 (Azeroth) for now; multi-map support is a
    // future change.
    if (pm_light_cycle) {
        pm_last_snapshot = pm_light_cycle->Sample(cam_pos, /*map_id=*/0u);
        pm_scene_data.pm_direct_color  = pm_last_snapshot.direct_color;
        pm_scene_data.pm_ambient_color = pm_last_snapshot.ambient_color;
        pm_scene_data.pm_fog_color     = pm_last_snapshot.sky_fog;
        pm_scene_data.pm_fog_end       = pm_last_snapshot.fog_distance;
        pm_scene_data.pm_fog_start     = pm_last_snapshot.fog_distance *
                                          pm_last_snapshot.fog_multiplier;
        // Recompute fog_rate from the new range. CalcFogRate:
        //   farClip = max(500, cameraFar - 200)  (we hardcode 500)
        //   range   = end - start
        //   rate    = (1 - range/farClip)*5.5 + 1.5  in [1.5, 7.0]
        const float far_clip = 500.0f;
        float range = pm_scene_data.pm_fog_end - pm_scene_data.pm_fog_start;
        float ratio = (range >= 0.0f && range <= far_clip) ? range / far_clip : 1.0f;
        pm_scene_data.pm_fog_rate = std::clamp((1.0f - ratio) * 5.5f + 1.5f,
                                                1.5f, 7.0f);
        pm_scene_data.pm_light_dir = pm_last_snapshot.light_dir;
    }

    pm_scene_ubo->Write(&pm_scene_data, sizeof(SceneUBO));

    // Extract frustum planes once - reused for terrain tile culling
    // and the doodad group bounding-sphere test below.
    glm::mat4 view_proj_for_cull =
        active_camera.GetProjectionMatrix() * active_camera.GetViewMatrix();
    auto frustum = ExtractFrustumPlanes(view_proj_for_cull);

    // Doodad draw radius. Past this distance a group is skipped
    // entirely. Slightly less than the fog end so anything we draw
    // still contributes a visible (if faded) pixel.
    constexpr float kDoodadCullDistance = 2600.0f;

    // Detail-grass cull radius. Grass blades become invisibly small
    // past ~50 yards (WoW's own client uses 60-80 depending on
    // settings) and the per-blade overdraw cost dominates at any
    // distance where you can't see individual blades. 60 yards
    // matches the canonical low-spec value and keeps the foreground
    // dense without paying for grass we couldn't see.
    constexpr float kGrassCullDistance = 60.0f;

    // Background gradient. The sky-cone shader takes 5 colors via
    // push constant: SkyTop, SkyMiddle, SkyBand1, SkyBand2 (rows 2-5
    // of LightIntBand) plus SkyFog (row 7) for the below-horizon
    // tint. When a LightCycle is attached, these come from the
    // current snapshot; when not, fall back to the constructor's
    // canonical-Elwynn-noon literals via pm_scene_data.pm_fog_color
    // and the hardcoded BGRA values below.
    struct BgPush {
        glm::vec4 sky_top;
        glm::vec4 sky_middle;
        glm::vec4 sky_band1;
        glm::vec4 sky_band2;
        glm::vec4 fog_color;
    };
    BgPush bg_push;
    if (pm_light_cycle) {
        bg_push.sky_top    = glm::vec4{pm_last_snapshot.sky_top,    0.0f};
        bg_push.sky_middle = glm::vec4{pm_last_snapshot.sky_middle, 0.0f};
        bg_push.sky_band1  = glm::vec4{pm_last_snapshot.sky_band1,  0.0f};
        bg_push.sky_band2  = glm::vec4{pm_last_snapshot.sky_band2,  0.0f};
        bg_push.fog_color  = glm::vec4{pm_last_snapshot.sky_fog,    0.0f};
    } else {
        bg_push = BgPush{
            glm::vec4{0.000f, 0.122f, 0.286f, 0.0f},
            glm::vec4{0.227f, 0.635f, 0.812f, 0.0f},
            glm::vec4{0.600f, 0.863f, 0.961f, 0.0f},
            glm::vec4{0.686f, 0.855f, 0.878f, 0.0f},
            glm::vec4{pm_scene_data.pm_fog_color, 0.0f},
        };
    }
    pm_bg_pipeline->Bind(cmd);
    cmd.pushConstants<BgPush>(
        *pm_bg_pipeline_layout, vk::ShaderStageFlagBits::eFragment, 0, bg_push);
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

    // Phase 2B placeholder player marker. Cube scaled to ~1.7 yards
    // tall (WoW player height) and centered ON the player's feet
    // (translate up by 0.85 = half-height). Uses the ground pipeline
    // since this is a primitive untextured colored mesh; gets the
    // same blended directional light + ambient as terrain so it
    // visually fits the scene.
    if (pm_player_visible) {
        glm::mat4 m = glm::translate(glm::mat4{1.0f},
                                      pm_player_pos + glm::vec3{0, 0.85f, 0});
        m = glm::scale(m, glm::vec3{0.6f, 1.7f, 0.6f});
        PushConstants player_push{};
        player_push.pm_model = m;
        player_push.pm_mvp = active_camera.GetProjectionMatrix() *
                              active_camera.GetViewMatrix() * m;
        cmd.pushConstants<PushConstants>(
            *pm_ground_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, player_push);

        pm_player_mesh.Bind(cmd);
        pm_player_mesh.Draw(cmd);
    }

    // Phase 2E stage 1: WMO bounding-box markers. Each WMO MODF
    // placement becomes one colored cube sized to fill the WMO's
    // AABB. Cheap proxy until the full WMO geometry loader lands -
    // tells us where buildings ARE before we can render their
    // detailed geometry.
    if (!pm_wmo_bboxes.empty()) {
        pm_player_mesh.Bind(cmd);   // reuse the unit cube
        for (const auto& m : pm_wmo_bboxes) {
            glm::mat4 mat = glm::translate(glm::mat4{1.0f},
                                            m.pos + glm::vec3{0.0f, m.extents.y * 0.5f, 0.0f});
            mat = glm::scale(mat, m.extents);
            PushConstants push{};
            push.pm_model = mat;
            push.pm_mvp   = active_camera.GetProjectionMatrix() *
                             active_camera.GetViewMatrix() * mat;
            cmd.pushConstants<PushConstants>(
                *pm_ground_pipeline_layout,
                vk::ShaderStageFlagBits::eVertex, 0, push);
            pm_player_mesh.Draw(cmd);
        }
    }

    // Terrain entities. Drawn before model entities so the depth buffer
    // has terrain depth written first; model entities (doodads in R4)
    // then test against it. Frustum culled by TerrainTileComponent's
    // centroid + radius (sphere test). At orbit 1500 with a 5x5 preload,
    // roughly 4-8 of the 25 tiles are visible in any direction - the
    // cull skips ~70% of terrain draws.
    pm_terrain_pipeline->Bind(cmd);
    scene.Each<TransformComponent, MeshComponent, TerrainComponent, TerrainTileComponent>(
        [&](Entity&, TransformComponent& transform, MeshComponent& mesh_comp,
            TerrainComponent& terrain, TerrainTileComponent& tile) {
            if (!mesh_comp.pm_visible || !mesh_comp.pm_mesh) return;
            if (!terrain.pm_descriptor_set) return;

            if (!SphereInFrustum(frustum, tile.pm_centroid_engine, tile.pm_radius)) {
                return;
            }

            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                *pm_terrain_pipeline_layout, 0,
                terrain.pm_descriptor_set, nullptr);

            glm::mat4 model = transform.ModelMatrix();
            PushConstants push{};
            push.pm_model = model;
            push.pm_mvp = view_proj_for_cull * model;
            cmd.pushConstants<PushConstants>(
                *pm_terrain_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, push);

            mesh_comp.pm_mesh->Bind(cmd);
            mesh_comp.pm_mesh->Draw(cmd);
        });

    // M2 pipeline. Bind once, then issue two passes over the scene:
    //
    //   Pass A - legacy M2 entities (MaterialComponent, no instancing).
    //   Used by the spell editor's per-model preview. One draw call per
    //   entity, push constants carry the full MVP, and the descriptor
    //   set's binding 3 is a 1-entry identity SSBO.
    //
    //   Pass B - instanced doodads (DoodadInstanceComponent). One draw
    //   call per unique M2 path, instance_count = number of MDDF
    //   placements. push.mvp carries only view*proj because the
    //   per-placement model matrix lives in the SSBO at binding 3.
    //
    // Pass A doesn't carry per-submesh blend modes yet (it's the legacy
    // single-material preview path used by the spell editor), so it
    // binds the alpha-key pipeline as a sensible default - that's the
    // pre-blend-mode behaviour and won't visibly change anything for
    // the editor preview meshes.
    pm_model_pipeline_alpha_key->Bind(cmd);

    scene.Each<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity& entity, TransformComponent& transform, MeshComponent& mesh_comp, MaterialComponent& mat) {
            if (!mesh_comp.pm_visible || !mesh_comp.pm_mesh) return;
            // Skip doodad entities here - the instanced pass below
            // handles them. An entity carrying both components means a
            // doodad with leftover material data from earlier (defensive).
            if (entity.HasComponent<DoodadInstanceComponent>()) return;

            cmd.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                *pm_model_pipeline_layout, 0,
                mat.pm_descriptor_set, nullptr);

            glm::mat4 model = transform.ModelMatrix();
            PushConstants push{};
            push.pm_model = model;
            push.pm_mvp = active_camera.GetProjectionMatrix() * active_camera.GetViewMatrix() * model;
            cmd.pushConstants<PushConstants>(
                *pm_model_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, push);

            mesh_comp.pm_mesh->Bind(cmd);
            mesh_comp.pm_mesh->Draw(cmd);
        });

    // Pass B: instanced doodads. The entity sits at the world origin
    // (Transform = identity), and the per-MDDF-placement TRS lives in
    // the SSBO that the vertex shader reads via gl_InstanceIndex. So
    // push.pm_model stays identity and push.pm_mvp = proj * view.
    //
    // Two-stage culling per group:
    //   1. Distance check from camera. Groups whose nearest possible
    //      instance is past kDoodadCullDistance are skipped entirely.
    //   2. Frustum sphere test on the group's coarse bbox.
    // Both bounds are loose (a group spanning many tiles has a huge
    // sphere), so the cull is conservative - cheap CPU, no correctness
    // risk.
    scene.Each<TransformComponent, MeshComponent, DoodadInstanceComponent>(
        [&](Entity&, TransformComponent&, MeshComponent& mesh_comp,
            DoodadInstanceComponent& inst) {
            if (!mesh_comp.pm_visible || !mesh_comp.pm_mesh) return;
            if (inst.pm_instance_count == 0) return;
            if (inst.pm_submeshes.empty()) return;

            float dist_to_nearest =
                glm::length(inst.pm_bbox_center - cam_pos) - inst.pm_bbox_radius;
            float cull_dist = inst.pm_is_detail_grass
                ? kGrassCullDistance : kDoodadCullDistance;
            if (dist_to_nearest > cull_dist) return;

            if (!SphereInFrustum(frustum, inst.pm_bbox_center, inst.pm_bbox_radius)) {
                return;
            }

            // Push constants are the same for every submesh - only the
            // bound pipeline, descriptor set, and index range change.
            PushConstants push{};
            push.pm_model = glm::mat4{1.0f};
            push.pm_mvp   = view_proj_for_cull;
            cmd.pushConstants<PushConstants>(
                *pm_model_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, push);

            mesh_comp.pm_mesh->Bind(cmd);

            // Sort submeshes so opaque + alpha-key (depth-writing)
            // draw first, then translucent (alpha / additive). Without
            // this, a mode-2 leaf-glow overlay drawn before a mode-1
            // canopy would have written depth and occluded the canopy
            // it's meant to overlay; or the mode-1 canopy would have
            // written depth that the mode-2 overlay then fails to
            // pass. Since pm_submeshes is small (1-7 entries per
            // tree), an in-place lambda sort is fine.
            //
            // Within each blend bucket we keep the original M2 batch
            // order (which the artists set up for correct overlay
            // stacking when modes match).
            struct SubmeshDrawOrder {
                int priority;        // 0 = opaque/alpha-key, 1 = translucent
                size_t original_idx; // stable tiebreak
            };
            std::vector<SubmeshDrawOrder> order;
            order.reserve(inst.pm_submeshes.size());
            for (size_t i = 0; i < inst.pm_submeshes.size(); ++i) {
                uint16_t bm = inst.pm_submeshes[i].pm_blend_mode;
                int prio = (bm == 0 || bm == 1) ? 0 : 1;
                order.push_back({prio, i});
            }
            std::sort(order.begin(), order.end(),
                [](const SubmeshDrawOrder& a, const SubmeshDrawOrder& b) {
                    if (a.priority != b.priority) return a.priority < b.priority;
                    return a.original_idx < b.original_idx;
                });

            for (const auto& ord : order) {
                const auto& sub = inst.pm_submeshes[ord.original_idx];
                if (!sub.pm_descriptor_set || sub.pm_index_count == 0) continue;

                // Pick + bind the pipeline whose blend / depth state
                // matches this submesh's blend_mode. Binding the same
                // pipeline back-to-back is essentially free on most
                // drivers; the explicit bind keeps the logic obvious.
                Pipeline* pipe = GetModelPipelineForBlendMode(sub.pm_blend_mode);
                pipe->Bind(cmd);
                // Push constants survive a pipeline rebind as long as
                // both pipelines use the same pipeline layout (they
                // do - pm_model_pipeline_layout is shared across all
                // variants). So we don't need to re-push.

                cmd.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    *pm_model_pipeline_layout, 0,
                    sub.pm_descriptor_set, nullptr);
                mesh_comp.pm_mesh->DrawInstancedRange(
                    cmd, inst.pm_instance_count,
                    sub.pm_index_start, sub.pm_index_count);
            }
        });

    pm_offscreen.EndRendering(cmd);
}

} // namespace mve
