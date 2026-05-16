#ifndef MVE_TERRAIN_MESH_H
#define MVE_TERRAIN_MESH_H

#include "mesh.h"
#include "../formats/adt_types.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace mve {

// Per-vertex layout for terrain. Distinct from the M2 Vertex so the M2
// pipeline isn't dragged into terrain-specific concerns (and so terrain
// can skip the bone-indices/weights it doesn't need).
//
// chunk_uv runs [0..1] across a single chunk. The terrain shader uses
// it to sample (a) the per-chunk slice of the alpha-map array texture
// and (b) the per-layer diffuse texture (tiled by a small constant).
//
// chunk_index is the linear MCNK index (0..255) within the tile. The
// shader uses it to index a per-chunk SSBO of layer-slot indices.
struct TerrainVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 chunk_uv;
    uint32_t  chunk_index;

    static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
};

// Per-chunk metadata uploaded to the terrain shader as an SSBO. The 4
// uints store the texture-array slice index for each of up to 4 layers,
// resolved from the tile's MTEX list. 0xFFFFFFFF marks unused slots so
// the shader can skip them.
struct TerrainChunkMeta {
    uint32_t layer_slot[4];
};

// Result of building a terrain mesh. The mesh itself is GPU-resident
// (vertex + index buffer). The companion blobs need to land in their
// own GPU resources (texture-array slices + SSBO) which the caller
// allocates via AssetManager / RenderSystem and binds in the
// TerrainComponent's descriptor set.
struct TerrainBuildResult {
    std::unique_ptr<Mesh> mesh;

    // Per-chunk metadata (256 entries). Packed RGBA8 alpha pixels
    // (256 slices * 64 * 64 * 4 bytes = 4 MB) with the chunk's three
    // upper-layer alpha maps in R/G/B (A reserved for MCSH later).
    std::vector<TerrainChunkMeta> chunk_meta;
    std::vector<uint8_t>          alpha_pixels;
};

class TerrainMesh {
public:
    // Build a terrain mesh from `tile`. Caller owns the returned struct.
    // `tile_tex_to_slice` maps a tile-relative MTEX index to a slice in
    // the texture array (typically built by
    // AssetManager::LoadAdtTextures). Unused layers get
    // layer_slot = 0xFFFFFFFF.
    static TerrainBuildResult Build(
        Device& device, const AdtTile& tile,
        const std::vector<int>& tile_tex_to_slice);
};

} // namespace mve

#endif // MVE_TERRAIN_MESH_H
