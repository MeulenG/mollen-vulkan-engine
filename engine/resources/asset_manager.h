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
    Entity* LoadAdtTileIntoScene(
        int tile_x, int tile_y, Scene& scene,
        const vk::raii::DescriptorSetLayout& terrain_layout);

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

    // Per-M2-path cached descriptor set. Used by doodads: instead of
    // each placement getting its own (mat, descriptor set, bone
    // buffer) triple, instances of the same M2 path share one. Cuts
    // pool consumption from ~4650 sets (one per placement) to ~50
    // (one per unique M2). Bone buffer is the identity matrix - fine
    // for static doodads (rocks, fences, signposts), wrong for
    // animated ones, but the editor's v1 acceptance is static-only.
    struct M2SharedMaterial {
        std::unique_ptr<Buffer> pm_bone_buffer;
        vk::DescriptorSet       pm_descriptor_set = VK_NULL_HANDLE;
        TextureHandle           pm_texture;
    };
    std::unordered_map<std::string, std::shared_ptr<M2SharedMaterial>>
        pm_shared_m2_material;

    // MDDF doodad unique_ids that have already been spawned across any
    // tile. Used to dedupe edge-shared doodads in the multi-tile R3
    // streamer: WoW places the same prop in multiple tiles' MDDF when
    // it sits near a tile boundary, so without dedup we'd render N
    // overlapping copies.
    std::unordered_set<uint32_t> pm_spawned_doodad_ids;

    // Descriptor resources (set externally by RenderSystem or main)
    const vk::raii::DescriptorSetLayout* pm_descriptor_layout = nullptr;
    DescriptorPool* pm_descriptor_pool = nullptr;
    const Buffer* pm_scene_ubo = nullptr;
};

} // namespace mve

#endif // MVE_ASSET_MANAGER_H
