#include "terrain_mesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <glm/glm.hpp>

namespace mve {

namespace {

// Half a chunk side in world units. The 9-vert outer grid spans the full
// chunk; the 8-vert inner grid is centered between them, half a cell off.
constexpr float kCellSize = kAdtChunkSize / 8.0f;   // distance between adjacent outer verts

// Convert WoW (south X, east Y, up Z) into engine (east X, up Y, south Z).
inline glm::vec3 WowToEngine(float wx, float wy, float wz) {
    return glm::vec3(wy, wz, wx);
}

// Compute a flat-shaded normal from three vertex positions in engine space.
inline glm::vec3 TriNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return glm::normalize(glm::cross(b - a, c - a));
}

// Resolve the layer slot for a chunk's layer index. Returns 0xFFFFFFFF
// when the layer is not used or when the texture isn't in the tile's
// texture list (which the array couldn't map).
uint32_t ResolveLayerSlot(const AdtChunk& ch, int layer_idx,
                          const std::vector<int>& tile_tex_to_slice) {
    if (layer_idx >= ch.layer_count) return 0xFFFFFFFFu;
    int tex_index = ch.layers[layer_idx].texture_index;
    if (tex_index < 0 ||
        static_cast<size_t>(tex_index) >= tile_tex_to_slice.size()) {
        return 0xFFFFFFFFu;
    }
    int slice = tile_tex_to_slice[tex_index];
    if (slice < 0) return 0xFFFFFFFFu;
    return static_cast<uint32_t>(slice);
}

} // namespace

std::vector<vk::VertexInputBindingDescription>
TerrainVertex::GetBindingDescriptions() {
    return {{0, sizeof(TerrainVertex), vk::VertexInputRate::eVertex}};
}

std::vector<vk::VertexInputAttributeDescription>
TerrainVertex::GetAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(TerrainVertex, position)},
        {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(TerrainVertex, normal)},
        {2, 0, vk::Format::eR32G32Sfloat,    offsetof(TerrainVertex, chunk_uv)},
        {3, 0, vk::Format::eR32Uint,         offsetof(TerrainVertex, chunk_index)},
    };
}

TerrainBuildResult TerrainMesh::Build(
    Device& device, const AdtTile& tile,
    const std::vector<int>& tile_tex_to_slice) {

    TerrainBuildResult result{};

    std::vector<TerrainVertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(static_cast<size_t>(kAdtChunksPerTile) * kAdtVertsPerChunk);
    indices.reserve(static_cast<size_t>(kAdtChunksPerTile) * 256 * 3);

    result.chunk_meta.resize(kAdtChunksPerTile);

    // 256 slices * 64 * 64 texels * 4 bytes (RGBA8). One slice per chunk.
    // R = layer 1 weight, G = layer 2 weight, B = layer 3 weight, A reserved.
    result.alpha_pixels.assign(
        static_cast<size_t>(kAdtChunksPerTile) * 64 * 64 * 4, 0);

    for (int cy = 0; cy < kAdtChunksPerSide; cy++) {
        for (int cx = 0; cx < kAdtChunksPerSide; cx++) {
            const int chunk_lin = cy * kAdtChunksPerSide + cx;
            const AdtChunk& ch = tile.chunks[chunk_lin];
            uint32_t base_v = static_cast<uint32_t>(vertices.size());

            // Per-chunk layer-slot metadata. The shader looks up the
            // 4 texture-array slices to sample from this.
            for (int l = 0; l < 4; l++) {
                result.chunk_meta[chunk_lin].layer_slot[l] =
                    ResolveLayerSlot(ch, l, tile_tex_to_slice);
            }

            // Pack this chunk's 3 upper-layer alpha maps into one
            // 64x64 RGBA8 slice of the global alpha array. Each row is
            // 64 texels * 4 bytes = 256 bytes; each slice is 64 rows.
            // Layer 0 has no alpha (always full coverage), so we start
            // from layer 1.
            uint8_t* slice =
                result.alpha_pixels.data() + chunk_lin * 64 * 64 * 4;
            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    uint8_t* px = slice + (y * 64 + x) * 4;
                    px[0] = (ch.layer_count > 1) ? ch.layers[1].alpha[y * 64 + x] : 0;
                    px[1] = (ch.layer_count > 2) ? ch.layers[2].alpha[y * 64 + x] : 0;
                    px[2] = (ch.layer_count > 3) ? ch.layers[3].alpha[y * 64 + x] : 0;
                    px[3] = 0;
                }
            }

            // Outer verts. Within the chunk, outer vertex (ox, oy) has
            // WoW-relative offsets (-oy * kCellSize, -ox * kCellSize) -
            // both axes are negative-step because WoW tiles increase
            // toward south-east while chunk-local coords increase toward
            // east-north (per wowdev wiki).
            for (int oy = 0; oy < 9; oy++) {
                for (int ox = 0; ox < 9; ox++) {
                    int idx = oy * 9 + ox;
                    float wx = ch.wow_x - oy * kCellSize;
                    float wy = ch.wow_y - ox * kCellSize;
                    float wz = ch.heights.y_outer[idx];

                    TerrainVertex v{};
                    v.position = WowToEngine(wx, wy, wz);
                    if (ch.normals.parsed) {
                        v.normal = glm::vec3(ch.normals.n_outer[idx * 3 + 0],
                                             ch.normals.n_outer[idx * 3 + 1],
                                             ch.normals.n_outer[idx * 3 + 2]);
                    } else {
                        v.normal = glm::vec3(0, 1, 0);  // refined below
                    }
                    v.chunk_uv    = glm::vec2(ox / 8.0f, oy / 8.0f);
                    v.chunk_index = static_cast<uint32_t>(chunk_lin);
                    vertices.push_back(v);
                }
            }
            // Inner verts (8x8, offset by 0.5 cell along both axes).
            for (int iy = 0; iy < 8; iy++) {
                for (int ix = 0; ix < 8; ix++) {
                    int idx = iy * 8 + ix;
                    float wx = ch.wow_x - (iy + 0.5f) * kCellSize;
                    float wy = ch.wow_y - (ix + 0.5f) * kCellSize;
                    float wz = ch.heights.y_inner[idx];

                    TerrainVertex v{};
                    v.position = WowToEngine(wx, wy, wz);
                    if (ch.normals.parsed) {
                        v.normal = glm::vec3(ch.normals.n_inner[idx * 3 + 0],
                                             ch.normals.n_inner[idx * 3 + 1],
                                             ch.normals.n_inner[idx * 3 + 2]);
                    } else {
                        v.normal = glm::vec3(0, 1, 0);
                    }
                    v.chunk_uv    = glm::vec2((ix + 0.5f) / 8.0f,
                                              (iy + 0.5f) / 8.0f);
                    v.chunk_index = static_cast<uint32_t>(chunk_lin);
                    vertices.push_back(v);
                }
            }

            // Index triangles. For each of 8x8 outer cells (ox, oy),
            // the cell's four outer corners are at outer indices:
            //   tl = oy*9 + ox       tr = oy*9 + ox + 1
            //   bl = (oy+1)*9 + ox   br = (oy+1)*9 + ox + 1
            // The inner vertex at the cell center is at index 81 + iy*8 + ix.
            for (int oy = 0; oy < 8; oy++) {
                for (int ox = 0; ox < 8; ox++) {
                    uint32_t tl = base_v + (uint32_t)(oy * 9 + ox);
                    uint32_t tr = base_v + (uint32_t)(oy * 9 + ox + 1);
                    uint32_t bl = base_v + (uint32_t)((oy + 1) * 9 + ox);
                    uint32_t br = base_v + (uint32_t)((oy + 1) * 9 + ox + 1);
                    uint32_t in = base_v + 81 + (uint32_t)(oy * 8 + ox);

                    // Four triangles fanning around the inner vertex.
                    // Winding chosen so the upward face's normal points +Y.
                    indices.push_back(in); indices.push_back(tr); indices.push_back(tl);
                    indices.push_back(in); indices.push_back(br); indices.push_back(tr);
                    indices.push_back(in); indices.push_back(bl); indices.push_back(br);
                    indices.push_back(in); indices.push_back(tl); indices.push_back(bl);
                }
            }
        }
    }

    // Fallback normal computation when MCNR wasn't parsed for any chunk:
    // run the R1 flat-aggregation pass. We don't know per-vertex whether
    // it came from a parsed chunk without extra bookkeeping, but the
    // fast path is "all chunks parsed", which is the common case.
    bool any_unparsed = false;
    for (const auto& ch : tile.chunks) {
        if (!ch.normals.parsed) { any_unparsed = true; break; }
    }
    if (any_unparsed) {
        // Zero normals we previously set to (0, 1, 0) and re-aggregate.
        for (auto& v : vertices) v.normal = glm::vec3(0);
        for (size_t i = 0; i + 3 <= indices.size(); i += 3) {
            uint32_t i0 = indices[i + 0];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];
            glm::vec3 n = TriNormal(vertices[i0].position,
                                    vertices[i1].position,
                                    vertices[i2].position);
            vertices[i0].normal += n;
            vertices[i1].normal += n;
            vertices[i2].normal += n;
        }
        for (auto& v : vertices) {
            if (glm::dot(v.normal, v.normal) > 0.0001f) {
                v.normal = glm::normalize(v.normal);
            } else {
                v.normal = glm::vec3(0, 1, 0);
            }
        }
    }

    result.mesh = std::make_unique<Mesh>(
        device,
        vertices.data(),
        sizeof(TerrainVertex) * vertices.size(),
        indices);
    return result;
}

} // namespace mve
