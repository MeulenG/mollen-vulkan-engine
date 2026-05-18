#include "grass_scatter.h"

#include "dbc_file.h"
#include "schemas/wotlk/schema_ground_effect_doodad.h"
#include "schemas/wotlk/schema_ground_effect_texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <vector>

namespace mve {

namespace {

// Slurp an entire file into a byte vector. Returns empty on failure -
// callers treat empty as a soft failure and disable the corresponding
// table.
std::vector<uint8_t> ReadFileBytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0) return {};
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (!in.read(reinterpret_cast<char*>(buf.data()), size)) return {};
    return buf;
}

// Deterministic per-cell hash. Same inputs always produce the same
// 32-bit value, so a tile reload doesn't shimmer. SplitMix64-style mix
// applied iteratively, finalised by a Murmur3-style avalanche.
inline uint32_t Hash32(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint64_t h = 0x9E3779B97F4A7C15ull;
    h ^= a; h *= 0xBF58476D1CE4E5B9ull; h ^= h >> 30;
    h ^= b; h *= 0x94D049BB133111EBull; h ^= h >> 27;
    h ^= c; h *= 0xBF58476D1CE4E5B9ull; h ^= h >> 31;
    h ^= d; h *= 0x94D049BB133111EBull;
    h ^= h >> 32;
    return static_cast<uint32_t>(h);
}

// Map a uint32 hash into [0, 1) by treating the high 24 bits as a
// fraction. 24 bits is plenty for jitter at sub-cell scale.
inline float HashUnit(uint32_t h) {
    return (h >> 8) * (1.0f / 16777216.0f);
}

// Bilinear sample of an MCNK 64x64 alpha map at the chunk_uv position
// (row_norm, col_norm) where row_norm is the east-axis fraction and
// col_norm is the south-axis fraction. Matches the terrain shader's
// sampling convention: texel address = col_idx * 64 + row_idx (the
// GLSL `texture(alpha, vec2(row_norm, col_norm))` lookup the shader
// uses resolves to this exact offset under the engine's row-major
// alpha storage).
inline float SampleAlpha(const uint8_t alpha[64 * 64],
                          float row_norm, float col_norm) {
    row_norm = std::min(std::max(row_norm, 0.0f), 1.0f);
    col_norm = std::min(std::max(col_norm, 0.0f), 1.0f);
    float fr = row_norm * 63.0f;
    float fc = col_norm * 63.0f;
    int r0 = static_cast<int>(std::floor(fr));
    int c0 = static_cast<int>(std::floor(fc));
    int r1 = std::min(r0 + 1, 63);
    int c1 = std::min(c0 + 1, 63);
    float tr = fr - r0;
    float tc = fc - c0;
    float a00 = alpha[c0 * 64 + r0] / 255.0f;
    float a10 = alpha[c0 * 64 + r1] / 255.0f;
    float a01 = alpha[c1 * 64 + r0] / 255.0f;
    float a11 = alpha[c1 * 64 + r1] / 255.0f;
    float a_c0 = a00 + (a10 - a00) * tr;
    float a_c1 = a01 + (a11 - a01) * tr;
    return a_c0 + (a_c1 - a_c0) * tc;
}

// Sample the chunk's height at a normalised position. row_norm is the
// east-axis fraction (0=N edge, 1=S edge in terms of chunk grid),
// col_norm is the south-axis fraction. The MCVT outer grid is stored
// as y_outer[row * 9 + col]; see terrain_mesh.cpp's mesh builder for
// the canonical layout. We bilinear-interpolate the 4 outer verts
// surrounding (row_norm, col_norm).
float SampleHeight(const AdtChunkHeights& heights,
                    float row_norm, float col_norm) {
    float fr = std::min(std::max(row_norm, 0.0f), 1.0f) * 8.0f;
    float fc = std::min(std::max(col_norm, 0.0f), 1.0f) * 8.0f;
    int r0 = static_cast<int>(std::floor(fr));
    int c0 = static_cast<int>(std::floor(fc));
    int r1 = std::min(r0 + 1, 8);
    int c1 = std::min(c0 + 1, 8);
    float tr = fr - r0;
    float tc = fc - c0;
    float h00 = heights.y_outer[r0 * 9 + c0];
    float h10 = heights.y_outer[r1 * 9 + c0];
    float h01 = heights.y_outer[r0 * 9 + c1];
    float h11 = heights.y_outer[r1 * 9 + c1];
    // Bilinear: interpolate along row direction first, then col.
    float h_c0 = h00 + (h10 - h00) * tr;
    float h_c1 = h01 + (h11 - h01) * tr;
    return h_c0 + (h_c1 - h_c0) * tc;
}

// Pick one of [0..n-1] weighted by `weights`. Returns -1 if the
// total weight is zero (no doodad assigned). `rand_unit` is a uniform
// float in [0, 1).
int WeightedPick(const uint32_t* weights, int n, float rand_unit) {
    uint32_t total = 0;
    for (int i = 0; i < n; i++) total += weights[i];
    if (total == 0) return -1;
    uint32_t r = static_cast<uint32_t>(rand_unit * total);
    if (r >= total) r = total - 1;
    uint32_t acc = 0;
    for (int i = 0; i < n; i++) {
        acc += weights[i];
        if (r < acc) return i;
    }
    return n - 1;
}

} // namespace

bool GroundEffectTables::Load(const std::string& texture_dbc_path,
                               const std::string& doodad_dbc_path) {
    pm_textures.clear();
    pm_doodads.clear();

    // --- GroundEffectDoodad ---
    auto doodad_bytes = ReadFileBytes(doodad_dbc_path);
    if (doodad_bytes.empty()) {
        std::fprintf(stderr,
            "GroundEffectTables: missing %s\n", doodad_dbc_path.c_str());
        return false;
    }
    DbcFile doodad_dbc;
    if (!doodad_dbc.Load(doodad_bytes.data(),
                          static_cast<uint32_t>(doodad_bytes.size()))) {
        std::fprintf(stderr,
            "GroundEffectTables: failed to parse %s\n",
            doodad_dbc_path.c_str());
        return false;
    }
    // Schema verification: 3 fields, 12 byte records.
    if (doodad_dbc.GetFieldCount() != schema_ground_effect_doodad.field_count ||
        doodad_dbc.GetRecordSize() != GetSchemaRecordSize(&schema_ground_effect_doodad)) {
        std::fprintf(stderr,
            "GroundEffectDoodad: schema mismatch (fields %u vs %u, size %u vs %u)\n",
            doodad_dbc.GetFieldCount(),
            schema_ground_effect_doodad.field_count,
            doodad_dbc.GetRecordSize(),
            GetSchemaRecordSize(&schema_ground_effect_doodad));
        return false;
    }
    pm_doodads.reserve(doodad_dbc.GetRecordCount());
    for (uint32_t r = 0; r < doodad_dbc.GetRecordCount(); r++) {
        GroundEffectDoodadRow row{};
        row.id    = doodad_dbc.GetUInt32(r, 0);
        // Field 1 is a string offset (DBC string field) - DoodadPath.
        // Field 2 is flags.
        const char* s = doodad_dbc.GetStringField(r, 1);
        row.path  = s ? std::string(s) : std::string();
        row.flags = doodad_dbc.GetUInt32(r, 2);
        if (row.id != 0) {
            pm_doodads.emplace(row.id, std::move(row));
        }
    }

    // --- GroundEffectTexture ---
    auto tex_bytes = ReadFileBytes(texture_dbc_path);
    if (tex_bytes.empty()) {
        std::fprintf(stderr,
            "GroundEffectTables: missing %s\n", texture_dbc_path.c_str());
        return false;
    }
    DbcFile tex_dbc;
    if (!tex_dbc.Load(tex_bytes.data(),
                       static_cast<uint32_t>(tex_bytes.size()))) {
        std::fprintf(stderr,
            "GroundEffectTables: failed to parse %s\n",
            texture_dbc_path.c_str());
        return false;
    }
    if (tex_dbc.GetFieldCount() != schema_ground_effect_texture.field_count ||
        tex_dbc.GetRecordSize() != GetSchemaRecordSize(&schema_ground_effect_texture)) {
        std::fprintf(stderr,
            "GroundEffectTexture: schema mismatch (fields %u vs %u, size %u vs %u)\n",
            tex_dbc.GetFieldCount(),
            schema_ground_effect_texture.field_count,
            tex_dbc.GetRecordSize(),
            GetSchemaRecordSize(&schema_ground_effect_texture));
        return false;
    }
    pm_textures.reserve(tex_dbc.GetRecordCount());
    for (uint32_t r = 0; r < tex_dbc.GetRecordCount(); r++) {
        GroundEffectTextureRow row{};
        row.id = tex_dbc.GetUInt32(r, 0);
        // Fields 1..4 = DoodadID1..4, 5..8 = DoodadWeight1..4,
        // 9 = Density, 10 = Sound.
        for (int i = 0; i < 4; i++) {
            row.doodad_ids[i]     = tex_dbc.GetUInt32(r, 1 + i);
            row.doodad_weights[i] = tex_dbc.GetUInt32(r, 5 + i);
        }
        row.density = tex_dbc.GetUInt32(r, 9);
        row.sound   = tex_dbc.GetUInt32(r, 10);
        if (row.id != 0) {
            pm_textures.emplace(row.id, row);
        }
    }

    std::fprintf(stderr,
        "GroundEffectTables: loaded %zu textures, %zu doodads\n",
        pm_textures.size(), pm_doodads.size());
    return true;
}

const GroundEffectTextureRow* GroundEffectTables::GetTexture(uint32_t id) const {
    auto it = pm_textures.find(id);
    return (it != pm_textures.end()) ? &it->second : nullptr;
}

const GroundEffectDoodadRow* GroundEffectTables::GetDoodad(uint32_t id) const {
    auto it = pm_doodads.find(id);
    return (it != pm_doodads.end()) ? &it->second : nullptr;
}

void ScatterGrassForTile(const AdtTile& tile,
                          const GroundEffectTables& tables,
                          int max_per_subcell,
                          std::vector<GrassPlacement>& out) {
    if (max_per_subcell <= 0) return;

    // Maximum count per (sub-cell, layer). The DBC Density field uses
    // "WoW units" that translate roughly to "doodads per chunk" - real
    // grass M2s in Elwynn cluster ~3-6 blades into one M2 file, so
    // even Density=12 only nets ~2 distinct placements per sub-cell.
    // We cap by max_per_subcell to keep memory bounded.

    for (int cy = 0; cy < kAdtChunksPerSide; cy++) {
        for (int cx = 0; cx < kAdtChunksPerSide; cx++) {
            const AdtChunk& ch = tile.chunks[cy * kAdtChunksPerSide + cx];
            if (ch.layer_count == 0) continue;

            // Cache effect rows for the chunk's up-to-4 layers. Most
            // chunks have 1-2 effect-bearing layers, the rest 0.
            const GroundEffectTextureRow* tex_rows[4] = {nullptr, nullptr, nullptr, nullptr};
            bool any_effect = false;
            for (int l = 0; l < ch.layer_count; l++) {
                uint32_t eid = ch.layers[l].effect_id;
                if (eid == 0) continue;
                tex_rows[l] = tables.GetTexture(eid);
                if (tex_rows[l]) any_effect = true;
            }
            if (!any_effect) continue;

            // Chunk NW corner in WoW coords. The mesh builder steps
            // through the chunk via:
            //   wx = ch.wow_x - col * kCellSize (col -> south)
            //   wy = ch.wow_y - row * kCellSize (row -> east)
            // So increasing (row, col) moves south-east in WoW frame.
            const float cell_yards = kAdtChunkSize / 8.0f;  // ~4.166 yards

            // 8x8 sub-cells per chunk. Each holds (row, col) in [0..7]
            // mapped via the chunk_uv convention used by the mesh
            // builder: chunk_uv.x = row/8 (east), chunk_uv.y = col/8
            // (south). Alpha maps are stored as alpha[y*64 + x] where
            // (x, y) follow the same row/col layout.
            for (int sr = 0; sr < 8; sr++) {
                for (int sc = 0; sc < 8; sc++) {
                    // Sample alpha at the sub-cell center. The +0.5
                    // offset puts the sample at the middle of the
                    // sub-cell which is also where we'll later place
                    // the grass.
                    float u = (sr + 0.5f) / 8.0f;   // east axis (row)
                    float v = (sc + 0.5f) / 8.0f;   // south axis (col)

                    // Per-layer raw weights. Layer 0 has no alpha map
                    // (full coverage minus whatever the upper layers
                    // overlay). We model layer 0 weight = clamp(1 -
                    // sum(upper)) so the four weights sum to ~1 even
                    // when MCAL doesn't fully obscure the base.
                    float upper_sum = 0.0f;
                    float lw[4] = {0, 0, 0, 0};
                    for (int l = 1; l < ch.layer_count; l++) {
                        lw[l] = SampleAlpha(ch.layers[l].alpha, u, v);
                        upper_sum += lw[l];
                    }
                    lw[0] = std::max(0.0f, 1.0f - upper_sum);

                    for (int l = 0; l < ch.layer_count; l++) {
                        if (!tex_rows[l]) continue;
                        const auto& trow = *tex_rows[l];
                        if (lw[l] < 0.10f) continue;   // < 10% coverage = skip

                        // Density math: WoW's Density field is in
                        // doodads-per-chunk per the wowdev wiki notes.
                        // We scale by the layer's local weight and cap
                        // by max_per_subcell. A 5%-covered layer with
                        // density=12 in this sub-cell gets:
                        //   12 * 0.05 / 64 = 0.009 -> floor=0, skip.
                        // A 100%-covered layer with density=12:
                        //   12 / 64 = 0.187 per sub-cell -> 1 every 5.
                        // We use a stochastic round so the average
                        // matches the analytic density.
                        float density_per_cell =
                            static_cast<float>(trow.density) *
                            lw[l] / 64.0f;
                        uint32_t base_hash = Hash32(
                            static_cast<uint32_t>(tile.tile_x),
                            static_cast<uint32_t>(tile.tile_y) * 13u +
                                static_cast<uint32_t>(cy) * 16u +
                                static_cast<uint32_t>(cx),
                            static_cast<uint32_t>(sr) * 8u +
                                static_cast<uint32_t>(sc),
                            static_cast<uint32_t>(l));
                        float r_density = HashUnit(base_hash);
                        int count = static_cast<int>(density_per_cell);
                        if (r_density < (density_per_cell - count)) count++;
                        if (count > max_per_subcell) count = max_per_subcell;
                        if (count <= 0) continue;

                        for (int d = 0; d < count; d++) {
                            uint32_t h = Hash32(
                                base_hash,
                                0xDEADBEEFu,
                                static_cast<uint32_t>(d),
                                0x12345678u);
                            float r_pick   = HashUnit(h);
                            float r_jitter_u = HashUnit(h * 1664525u + 1013904223u);
                            float r_jitter_v = HashUnit(h * 22695477u + 1u);
                            float r_rot      = HashUnit(h * 134775813u + 1u);
                            float r_scale    = HashUnit(h * 2654435761u);

                            int slot = WeightedPick(trow.doodad_weights, 4, r_pick);
                            if (slot < 0) continue;
                            uint32_t did = trow.doodad_ids[slot];
                            if (did == 0) continue;
                            const auto* drow = tables.GetDoodad(did);
                            if (!drow || drow->path.empty()) continue;

                            // Jittered cell position. Stay inside the
                            // sub-cell by clamping the jitter to about
                            // 80% of the cell size so blades on the
                            // edge don't appear to cross into the
                            // neighbouring sub-cell.
                            float ju = (r_jitter_u - 0.5f) * 0.8f;  // [-0.4, 0.4]
                            float jv = (r_jitter_v - 0.5f) * 0.8f;
                            float row_norm = (sr + 0.5f + ju) / 8.0f;
                            float col_norm = (sc + 0.5f + jv) / 8.0f;
                            // Same WoW-coord stepping as the mesh
                            // builder so the grass sits exactly on the
                            // rendered terrain surface.
                            float wow_x = ch.wow_x - col_norm * kAdtChunkSize;  // south
                            float wow_y = ch.wow_y - row_norm * kAdtChunkSize;  // east
                            float wow_z = SampleHeight(ch.heights, row_norm, col_norm);

                            // WoW->engine remap (east, up, south).
                            glm::vec3 engine_pos(wow_y, wow_z, wow_x);

                            // Random Y rotation (about engine up axis)
                            // for visual variety. The DBC flag bit 0x1
                            // is "rotate per instance" but in practice
                            // every grass M2 wants rotation, so we
                            // always randomize.
                            float rot_y = r_rot * 6.28318530718f;
                            // Modest scale jitter so a field of the
                            // same M2 doesn't look like a regular
                            // grid.
                            float scale = 0.85f + r_scale * 0.30f;  // 0.85..1.15

                            // M2 doodads in WoW are authored Z-up;
                            // doodad placement also applies a -90 deg
                            // rotation about the WoW X axis to bring
                            // them to engine Y-up. We do the same
                            // here so grass blades stand vertical.
                            glm::mat4 m =
                                glm::translate(glm::mat4{1.0f}, engine_pos)
                              * glm::rotate(glm::mat4{1.0f}, rot_y,
                                             glm::vec3{0.0f, 1.0f, 0.0f})
                              * glm::rotate(glm::mat4{1.0f},
                                             glm::radians(-90.0f),
                                             glm::vec3{1.0f, 0.0f, 0.0f})
                              * glm::scale(glm::mat4{1.0f}, glm::vec3{scale});

                            GrassPlacement p{};
                            p.wow_m2_path = drow->path;
                            p.model_matrix = m;
                            out.push_back(std::move(p));
                        }
                    }
                }
            }
        }
    }
}

} // namespace mve
