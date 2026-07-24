#ifndef MVE_TERRAIN_COMPONENT_H
#define MVE_TERRAIN_COMPONENT_H

#include "../entity.h"
#include "../../resources/buffer.h"
#include "../../resources/texture_array.h"

#include <memory>

namespace mve {

// A renderable terrain tile. Distinct from MaterialComponent so the
// terrain pipeline can query Each<TransformComponent, MeshComponent,
// TerrainComponent> without dragging in the M2 material concepts
// (per-submesh blend modes, bone buffer, etc.) that terrain doesn't use.
//
// Bindings consumed by the terrain pipeline:
//   set=0 binding=0  pm_scene_ubo (shared)        - UBO
//   set=0 binding=1  pm_chunk_meta_ssbo           - SSBO (256 * 4 uints)
//   set=0 binding=2  pm_diffuse_array             - 2D-array sampler
//   set=0 binding=3  pm_alpha_array               - 2D-array sampler
struct TerrainComponent : Component {
    std::shared_ptr<TextureArray> pm_diffuse_array;
    std::shared_ptr<TextureArray> pm_alpha_array;
    std::unique_ptr<Buffer>       pm_chunk_meta_ssbo;
    vk::raii::DescriptorSet       pm_descriptor_set = nullptr;
};

} // namespace mve

#endif // MVE_TERRAIN_COMPONENT_H
