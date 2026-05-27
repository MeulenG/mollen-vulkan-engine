#ifndef MVE_TERRAIN_COMPONENT_H
#define MVE_TERRAIN_COMPONENT_H

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
struct TerrainComponent {
    std::shared_ptr<TextureArray> pm_diffuse_array;
    std::shared_ptr<TextureArray> pm_alpha_array;
    std::unique_ptr<Buffer>       pm_chunk_meta_ssbo;

    // Raw handle (not vk::raii) - we deliberately do not free
    // individual descriptor sets when entities are destroyed.
    //
    // History: an earlier attempt used vk::raii::DescriptorSet for
    // automatic cleanup, but its destructor reliably triggered
    // 'Invalid VkDescriptorPool Object' validation errors and driver
    // crashes during R3 tile eviction even with waitIdle guards. The
    // exact cause is unclear (looks like double-free or stale handle
    // copy under move-assignment on Intel's Vulkan stack), and the
    // diagnostic effort to chase it isn't justified for an editor
    // where the entire pool is reclaimed at shutdown anyway.
    //
    // The pool is sized for 8192 entities; flying across ~150 unique
    // tiles in one session exhausts that, which is a separate concern
    // (instancing or static-doodad packing will fix it).
    vk::DescriptorSet pm_descriptor_set = VK_NULL_HANDLE;
};

} // namespace mve

#endif // MVE_TERRAIN_COMPONENT_H
