#include "water_mesh.h"

#include <cstring>
#include <glm/glm.hpp>

namespace mve {

namespace {

// One 8x8 sub-cell of an MCNK in WoW yards. MCNK side is kAdtChunkSize
// (~33.33 yd), the liquid grid divides each MCNK into 8 cells per side,
// matching the inner-grid spacing of the terrain heightmap exactly.
constexpr float kCellSize = kAdtChunkSize / 8.0f;

// Planar UV scale used by WoW's water shader for LVF=0 (Elwynn river).
// 0.06 yd^-1 = texture repeats every 16.67 yards (per wowdev.wiki notes
// embedded in LiquidRenderer.js). Picks a comfortable texel-per-yard
// density at WoW's ~2yd player-eye height.
constexpr float kPlanarUvScale = 0.06f;

inline bool ExistsBitSet(const AdtLiquidInstance& inst, uint32_t lx, uint32_t ly) {
    if (inst.exists_bits.empty()) return true;   // 0 offset = "all exist"
    uint32_t bit = ly * inst.width + lx;
    return (inst.exists_bits[bit >> 3] >> (bit & 7)) & 1u;
}

} // namespace

std::vector<vk::VertexInputBindingDescription>
WaterVertex::GetBindingDescriptions() {
    return {{0, sizeof(WaterVertex), vk::VertexInputRate::eVertex}};
}

std::vector<vk::VertexInputAttributeDescription>
WaterVertex::GetAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(WaterVertex, position)},
        {1, 0, vk::Format::eR32G32Sfloat,    offsetof(WaterVertex, uv)},
        {2, 0, vk::Format::eR32Sfloat,       offsetof(WaterVertex, depth)},
    };
}

std::unique_ptr<Mesh> WaterMesh::Build(Device& device,
                                       const AdtTile& tile,
                                       const AdtLiquidInstance& inst) {
    if (inst.width == 0 || inst.height == 0) return nullptr;
    if (inst.chunk_index >= kAdtChunksPerTile) return nullptr;

    const AdtChunk& ch = tile.chunks[inst.chunk_index];

    // Vertex count is (width+1) * (height+1) regardless of LVF (per
    // wowdev.wiki/ADT/v18 SMLiquidInstance). Allocate the full grid
    // even if some triangles are excluded by the exists bitmap -
    // interior bitmap-excluded triangles still reference the surrounding
    // vertices.
    const uint32_t vw = inst.width + 1u;
    const uint32_t vh = inst.height + 1u;
    const uint32_t n_verts = vw * vh;

    std::vector<WaterVertex> verts;
    verts.reserve(n_verts);

    for (uint32_t ly = 0; ly < vh; ++ly) {
        for (uint32_t lx = 0; lx < vw; ++lx) {
            // Convert local (lx, ly) cell to WoW world coordinates. Same
            // axis-swap convention as TerrainMesh:
            //   column index (lx, x_offset) drives WoW X (south axis)
            //   row index    (ly, y_offset) drives WoW Y (east axis)
            // and each step is one cell (~4.166 yd).
            float wow_x = ch.wow_x - (inst.x_offset + lx) * kCellSize;
            float wow_y = ch.wow_y - (inst.y_offset + ly) * kCellSize;
            float wow_z = inst.heights[ly * vw + lx];

            WaterVertex v{};
            // WowToEngine: (south, east, up) -> (east, up, south).
            v.position = glm::vec3{wow_y, wow_z, wow_x};
            // Planar UV from world x/y. Using WoW-space coords here so
            // the texture stays world-aligned regardless of camera.
            v.uv    = glm::vec2{wow_x * kPlanarUvScale,
                                 wow_y * kPlanarUvScale};
            v.depth = inst.depths[ly * vw + lx];
            verts.push_back(v);
        }
    }

    // Emit triangles per CELL (not per vertex), gated by the exists
    // bitmap. Each cell makes two triangles sharing the diagonal
    // tl-br. Indices reference verts in row-major order:
    //
    //   tl = ly*vw + lx
    //   tr = tl + 1
    //   bl = tl + vw
    //   br = bl + 1
    //
    // Winding: water surface normal points +Y (up) in engine space.
    // With ly increasing toward decreasing WoW Y (east) and after the
    // WowToEngine swap, the CCW order seen from above is (tl, bl, tr)
    // for the first triangle and (tr, bl, br) for the second.
    // Pipeline runs with cullMode=eNone anyway (so the player swimming
    // under the surface still sees water from below), but consistent
    // winding lets us re-enable culling later without re-meshing.
    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(inst.width) * inst.height * 6u);

    for (uint32_t cy = 0; cy < inst.height; ++cy) {
        for (uint32_t cx = 0; cx < inst.width; ++cx) {
            if (!ExistsBitSet(inst, cx, cy)) continue;
            uint32_t tl = cy * vw + cx;
            uint32_t tr = tl + 1;
            uint32_t bl = tl + vw;
            uint32_t br = bl + 1;
            indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
            indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
        }
    }

    if (indices.empty()) return nullptr;   // bitmap excluded everything

    return std::make_unique<Mesh>(
        device,
        verts.data(),
        verts.size() * sizeof(WaterVertex),
        indices);
}

} // namespace mve
