#include "terrain_mesh.h"

#include <algorithm>
#include <cmath>
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

// Pick a vertex color based on height. Stand-in for real texture splatting
// in R1 - lets you see the terrain's shape immediately. Greens for low,
// browns for mid, whites for high.
glm::vec3 ColorForHeight(float h) {
    if (h < 30.0f)  return glm::vec3(0.30f, 0.55f, 0.25f);  // grass
    if (h < 80.0f)  return glm::vec3(0.45f, 0.40f, 0.20f);  // dirt
    if (h < 150.0f) return glm::vec3(0.55f, 0.50f, 0.45f);  // rock
    return                 glm::vec3(0.95f, 0.95f, 0.95f);  // snow
}

// Compute a flat-shaded normal from three vertex positions in engine space.
inline glm::vec3 TriNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return glm::normalize(glm::cross(b - a, c - a));
}

} // namespace

std::unique_ptr<Mesh> TerrainMesh::Build(Device& device, const AdtTile& tile) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Pre-reserve. Each chunk contributes 145 verts (9*9 outer + 8*8 inner)
    // and 256 triangles (768 indices).
    vertices.reserve(static_cast<size_t>(kAdtChunksPerTile) * kAdtVertsPerChunk);
    indices.reserve(static_cast<size_t>(kAdtChunksPerTile) * 256 * 3);

    // Each chunk emits its 145 vertices contiguously. Within a chunk:
    //   Indices 0..80   = 9x9 outer grid, row-major (9 per row, 9 rows)
    //   Indices 81..144 = 8x8 inner grid, row-major (8 per row, 8 rows)
    // For each of the 8x8 outer cells we fan from the corresponding inner
    // vertex to the four corners (4 triangles).
    for (int cy = 0; cy < kAdtChunksPerSide; cy++) {
        for (int cx = 0; cx < kAdtChunksPerSide; cx++) {
            const AdtChunk& ch = tile.chunks[cy * kAdtChunksPerSide + cx];
            uint32_t base_v = static_cast<uint32_t>(vertices.size());

            // Outer verts. Within the chunk, outer vertex (ox, oy) has
            // WoW-relative offsets (-ox * kCellSize, -oy * kCellSize) -
            // both axes are negative-step because WoW tiles increase
            // toward south-east while chunk-local coords increase toward
            // east-north (per wowdev wiki, but easier to verify visually).
            for (int oy = 0; oy < 9; oy++) {
                for (int ox = 0; ox < 9; ox++) {
                    int idx = oy * 9 + ox;
                    float wx = ch.wow_x - oy * kCellSize;
                    float wy = ch.wow_y - ox * kCellSize;
                    float wz = ch.heights.y_outer[idx];
                    Vertex v{};
                    v.position = WowToEngine(wx, wy, wz);
                    v.normal   = glm::vec3(0, 1, 0);  // refined below
                    v.color    = ColorForHeight(wz);
                    v.uv       = glm::vec2(ox / 8.0f, oy / 8.0f);
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
                    Vertex v{};
                    v.position = WowToEngine(wx, wy, wz);
                    v.normal   = glm::vec3(0, 1, 0);
                    v.color    = ColorForHeight(wz);
                    v.uv       = glm::vec2((ix + 0.5f) / 8.0f, (iy + 0.5f) / 8.0f);
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

    // Smooth-ish normals via flat-shading aggregation: for each triangle,
    // accumulate its face normal onto each of its three vertices, then
    // normalize at the end. Cheap and good enough for v1.
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

    return std::make_unique<Mesh>(device, vertices, indices);
}

} // namespace mve
