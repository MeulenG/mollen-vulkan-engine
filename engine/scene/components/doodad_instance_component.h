#ifndef MVE_DOODAD_INSTANCE_COMPONENT_H
#define MVE_DOODAD_INSTANCE_COMPONENT_H

#include "../entity.h"
#include "../../resources/buffer.h"

#include <cstdint>
#include <memory>

namespace mve {

// One entity per unique M2 doodad path. The entity holds a single
// shared mesh (in MeshComponent) plus this component, which carries
// the per-instance model matrices in a host-visible SSBO. The render
// system issues a single drawIndexed(idx_count, instance_count, ...)
// per entity instead of one draw per placement.
//
// Why: a 5x5 Elwynn preload spawns ~4650 MDDF doodads. Per-placement
// entities means ~4650 draw calls = ~14 ms just in API overhead even
// before the GPU touches anything. After instancing, we get ~50 draw
// calls (one per unique M2 path) for the same scene.
//
// Bindings consumed by the M2 pipeline:
//   set=0 binding=0  scene UBO        (shared)
//   set=0 binding=1  diffuse texture  (shared per M2 path)
//   set=0 binding=2  bone matrices    (identity for static doodads)
//   set=0 binding=3  instance models  <-- this component owns it
//
// pm_descriptor_set is a raw vk::DescriptorSet handle - mirrors the
// convention in MaterialComponent / TerrainComponent (the pool
// reclaims everything at shutdown; per-entity raii destruction is
// unreliable on this driver stack). It's shared across all instances
// of one M2 path - the AssetManager caches it in pm_shared_m2_material.
// One entry per M2 batch (= drawcall). Each carries its own descriptor
// set (bound texture + bone buffer + instance buffer) and an index
// range into the shared mesh's index buffer.
//
// pm_blend_mode and pm_render_flags drive per-submesh pipeline
// selection at draw time. See M2Material:
//   blend_mode (uint16_t):
//     0 = Opaque        (no alpha test, no blend, depth write)
//     1 = AlphaKey      (alpha test discard at 128/255, depth write)
//     2 = Alpha         (SrcAlpha/OneMinusSrcAlpha blend, no depth write)
//     3 = Add           (SrcAlpha/One additive, no depth write)
//     4 = Mod, 5 = Mod2x, 6 = ModAdd, 7 = InvSrcAlphaAdd
//   render_flags (uint16_t bitfield):
//     0x01 = unlit, 0x02 = unfogged, 0x04 = two-sided,
//     0x08 = depth-test off, 0x10 = depth-write off
//
// A typical Elwynn tree M2 has 2 submeshes: one mode-0 trunk (opaque
// bark, gets full color contribution) and one mode-1 canopy (alpha-
// keyed leaf planes). Pushing everything through a single alpha-key
// pipeline as we did pre-this-change discarded trunk pixels with
// alpha < threshold (BC1-no-alpha samples returning alpha=1 made the
// discard never fire so trunks did render, but additive overlays on
// other doodads such as sword glows, magic effects, and water-edge
// foam never got their proper blend math).
struct DoodadSubmesh {
    vk::DescriptorSet pm_descriptor_set = VK_NULL_HANDLE;
    uint32_t          pm_index_start = 0;
    uint32_t          pm_index_count = 0;
    uint16_t          pm_blend_mode   = 0;
    uint16_t          pm_render_flags = 0;
};

struct DoodadInstanceComponent : Component {
    std::unique_ptr<Buffer>    pm_instance_buffer;  // SSBO of mat4 transforms
    uint32_t                   pm_instance_count = 0;
    std::vector<DoodadSubmesh> pm_submeshes;

    // Coarse bounding sphere covering every instance's world position.
    // Updated when instances are added; used by RenderSystem for a
    // cheap centroid-distance cull (skip the whole group if its nearest
    // possible instance is farther than the doodad draw radius).
    glm::vec3 pm_bbox_center{0.0f};
    float     pm_bbox_radius{0.0f};
};

} // namespace mve

#endif // MVE_DOODAD_INSTANCE_COMPONENT_H
