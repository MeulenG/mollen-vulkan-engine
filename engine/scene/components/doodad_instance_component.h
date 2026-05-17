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
struct DoodadInstanceComponent : Component {
    std::unique_ptr<Buffer> pm_instance_buffer;  // SSBO of mat4 transforms
    vk::DescriptorSet       pm_descriptor_set = VK_NULL_HANDLE;
    uint32_t                pm_instance_count = 0;
};

} // namespace mve

#endif // MVE_DOODAD_INSTANCE_COMPONENT_H
