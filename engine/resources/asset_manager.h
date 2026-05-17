#ifndef MVE_ASSET_MANAGER_H
#define MVE_ASSET_MANAGER_H

#include "../core/device.h"
#include "../scene/mesh.h"
#include "../scene/scene.h"
#include "../resources/image.h"
#include "../resources/descriptor.h"
#include "../resources/texture_array.h"
#include "../formats/m2_loader.h"
#include "../formats/adt_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace mve {

using MeshHandle = std::shared_ptr<Mesh>;
using TextureHandle = std::shared_ptr<Image>;

// Result of loading a tile's diffuse textures into a 2D-array image.
// tile_tex_to_slice[i] gives the array slice for the i-th MTEX entry
// in the parent ADT tile, or -1 if that texture failed to load.
struct AdtTextureSet {
    std::unique_ptr<TextureArray> diffuse;
    std::vector<int> tile_tex_to_slice;
};

class AssetManager {
public:
    AssetManager(Device& device);

    // Load an M2 model and spawn a fully-wired entity in the scene.
    // Caches M2 data and textures so loading the same model twice
    // shares GPU resources. Uses the legacy WoW-coord transform that
    // hoists the model so its lowest bbox corner sits at y=0.
    Entity* LoadM2IntoScene(const std::string& m2_path, Scene& scene);

    // Same, but with an explicit world transform - used by R4 doodad
    // spawning where the MDDF data specifies position + Euler rotation
    // + scale directly. The caller is responsible for converting WoW
    // coords to engine coords before passing them in.
    //
    // name_hint becomes the entity's debug name (typically
    // "Doodad#<unique_id>" so the editor entity panel can identify it).
    // Empty name_hint falls back to the model's internal name.
    Entity* LoadM2IntoScene(
        const std::string& m2_path, Scene& scene,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3& scale,
        const std::string& name_hint = "");

    // Load one ADT terrain tile (parses Azeroth_<x>_<y>.adt under
    // assets/World/Maps/Azeroth/), build its mesh + diffuse/alpha
    // atlases, and spawn a fully-wired Transform + Mesh + Terrain +
    // TerrainTile entity in the scene. The caller supplies the
    // terrain-pipeline descriptor layout because RenderSystem owns it.
    //
    // Returns nullptr if the .adt file doesn't exist or fails to parse.
    // The mesh is cached by "adt:<x>_<y>" so re-loading a tile shares
    // GPU memory; BLPs are cached per-path inside LoadAdtTextures.
    //
    // R4.5: doodad MDDF entries are NOT spawned per-placement. Instead
    // the tile loader accumulates them into pm_pending_doodad_instances
    // (keyed by M2 path) and the caller must invoke FlushDoodadInstances
    // once after all desired tiles are loaded. This collapses ~4650
    // draw calls into ~50 (one per unique M2 path) for a 5x5 preload.
    Entity* LoadAdtTileIntoScene(
        int tile_x, int tile_y, Scene& scene,
        const vk::raii::DescriptorSetLayout& terrain_layout);

    // Materialize pending doodad placements (accumulated by
    // LoadAdtTileIntoScene) into one entity per unique M2 path. Each
    // entity gets MeshComponent + DoodadInstanceComponent and is drawn
    // via vkCmdDrawIndexed(idx_count, instance_count, ...).
    //
    // Safe to call multiple times - it only consumes whatever is
    // pending and clears it. Typical flow:
    //   for tile in (tx, ty) ... LoadAdtTileIntoScene(...)
    //   FlushDoodadInstances(scene)
    //
    // The pending list is shared across tiles so cross-tile dedup works
    // (pm_spawned_doodad_ids tracks unique_ids).
    void FlushDoodadInstances(Scene& scene);

    // Load the diffuse BLP textures referenced by an ADT tile's MTEX
    // list into a 2D-array image. Each unique BLP gets one slice.
    // BLPs that fail to load (missing file, format mismatch with the
    // first valid BLP, etc.) get tile_tex_to_slice[i] = -1; consumers
    // typically map these to a checkerboard fallback.
    //
    // target_size is the per-slice width/height (square). The loader
    // picks the appropriate mip out of each BLP. Typical value: 256.
    AdtTextureSet LoadAdtTextures(const AdtTile& tile,
                                   uint32_t target_size = 256);

    // Set the descriptor layout + pool that entities will use.
    // Must be called before loadM2IntoScene.
    void SetDescriptorResources(
        const vk::raii::DescriptorSetLayout& layout,
        DescriptorPool& pool,
        const Buffer& scene_ubo);

    MeshHandle GetMesh(const std::string& key) const;
    TextureHandle GetTexture(const std::string& key) const;
    TextureHandle GetDefaultTexture();

    // Exposed for callers that need to synchronize with the GPU before
    // freeing resources (e.g. TerrainStreamer eviction must waitIdle
    // before destroying a tile entity whose descriptor set might still
    // be referenced by an in-flight command buffer).
    Device& GetDevice() { return pm_device; }

private:
    Device& pm_device;

    // Cached parsed M2 data (skeleton + animations shared across instances)
    struct M2CacheEntry {
        M2Model model;
    };
    std::unordered_map<std::string, std::shared_ptr<M2CacheEntry>> pm_m2_cache;
    std::unordered_map<std::string, MeshHandle> pm_mesh_cache;
    std::unordered_map<std::string, TextureHandle> pm_texture_cache;
    TextureHandle pm_default_texture;

    // Per-M2-path cached descriptor set + shared bone buffer. Used by
    // both the legacy single-arg LoadM2IntoScene (one-off previews) and
    // the doodad path. R4.5 also caches a single shared identity-instance
    // buffer for the legacy path so binding 3 of the descriptor set is
    // always populated even when there's no real instance data.
    //
    // Bone buffer is the identity matrix - fine for static doodads
    // (rocks, fences, signposts), wrong for animated ones, but the
    // editor's v1 acceptance is static-only.
    struct M2SharedMaterial {
        std::unique_ptr<Buffer> pm_bone_buffer;
        vk::DescriptorSet       pm_descriptor_set = VK_NULL_HANDLE;
        TextureHandle           pm_texture;
    };
    std::unordered_map<std::string, std::shared_ptr<M2SharedMaterial>>
        pm_shared_m2_material;

    // 1-entry "identity" instance buffer used by the legacy M2 path so
    // binding 3 of the M2 descriptor set is always populated. Created
    // lazily on first legacy LoadM2IntoScene call. The shader reads
    // gl_InstanceIndex=0 -> identity matrix, so the legacy math collapses
    // to the pre-R4.5 behaviour (push.mvp * identity * skinned_pos).
    std::unique_ptr<Buffer> pm_identity_instance_buffer;

    // MDDF doodad unique_ids that have already been spawned across any
    // tile. Used to dedupe edge-shared doodads in the multi-tile R3
    // streamer: WoW places the same prop in multiple tiles' MDDF when
    // it sits near a tile boundary, so without dedup we'd render N
    // overlapping copies.
    std::unordered_set<uint32_t> pm_spawned_doodad_ids;

    // Doodad placements parked by LoadAdtTileIntoScene, awaiting flush.
    // Keyed by resolved M2 path so multiple tiles' placements of the
    // same model coalesce into one entity at flush time. Each entry is
    // a TRS already converted to engine space.
    struct PendingDoodadInstance {
        glm::mat4 model_matrix;
    };
    std::unordered_map<std::string, std::vector<PendingDoodadInstance>>
        pm_pending_doodad_instances;

    // ALL placements ever flushed for a given M2 path, in instance-buffer
    // order. Used by FlushDoodadInstances to extend existing entities
    // when new tiles stream in: we rebuild the SSBO with old + new so
    // the existing entity's instance_count grows rather than the scene
    // accumulating one entity per flush call.
    //
    // Doodads are never evicted in v1 - they live for the lifetime of
    // the AssetManager. Tile eviction only frees terrain meshes/textures.
    std::unordered_map<std::string, std::vector<glm::mat4>>
        pm_flushed_doodad_matrices;
    std::unordered_map<std::string, EntityId>
        pm_doodad_entity_for_path;

    // Descriptor resources (set externally by RenderSystem or main)
    const vk::raii::DescriptorSetLayout* pm_descriptor_layout = nullptr;
    DescriptorPool* pm_descriptor_pool = nullptr;
    const Buffer* pm_scene_ubo = nullptr;
};

} // namespace mve

#endif // MVE_ASSET_MANAGER_H
