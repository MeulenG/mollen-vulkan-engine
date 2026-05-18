#ifndef MVE_ADT_TYPES_H
#define MVE_ADT_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

namespace mve {

// ADT (terrain) tile constants. A tile covers ~533 yards square divided
// into 16x16 = 256 MCNK chunks. Each MCNK is 33.33 yards on a side.
// Each MCNK stores its heightmap as 145 vertices arranged as
//   - an outer 9x9 grid (81 verts) on integer coordinates
//   - an inner 8x8 grid (64 verts) at the centers of the 9x9 cells, slightly raised
// The triangle layout uses the inner verts as fans to the four corners of
// each outer cell - 4 triangles per outer cell - so each MCNK produces
// 8*8*4 = 256 triangles.
constexpr int kAdtChunksPerSide       = 16;        // 16x16 MCNK chunks per ADT tile
constexpr int kAdtChunksPerTile       = 256;       // 16*16
constexpr int kAdtVertsPerChunk       = 145;       // 9*9 outer + 8*8 inner
constexpr float kAdtChunkSize         = 100.0f / 3.0f;  // 33.333... yards per MCNK side
constexpr float kAdtTileSize          = 16.0f * kAdtChunkSize;  // 533.33 yards
constexpr float kAdtMaxCoord          = 17066.0f + 2.0f/3.0f;   // map midpoint in WoW coords

// Per-chunk heightmap. y_outer[0..80] is the 9x9 outer grid (row-major,
// 9 columns per row). y_inner[0..63] is the 8x8 inner grid.
// All values are absolute heights in world units (the MCNK's base height
// has already been folded in by the loader).
struct AdtChunkHeights {
    float y_outer[81];   // 9x9
    float y_inner[64];   // 8x8
};

// Per-vertex unit normals in engine space (already WowToEngine-remapped).
// 145 entries packed as 9x9 outer (81) followed by 8x8 inner (64). The
// shape mirrors AdtChunkHeights so vertex emission can index in lockstep.
//
// `parsed` is false when MCNR was absent or malformed; the mesh builder
// then falls back to cross-product-summed normals as in R1.
struct AdtChunkNormals {
    float n_outer[81 * 3];
    float n_inner[64 * 3];
    bool  parsed = false;
};

// Optional MCCV - per-vertex RGB tint applied to terrain shading. WoW
// uses this to paint regional color variation (sun-bleached patches,
// darker rocky outcrops, mossy areas) that the splat textures alone
// can't express. 145 RGB triplets stored as 0..255 -> 0..1 floats.
//
// `parsed` is false when MCCV was absent (older ADTs / no painted
// tints); the mesh builder then uses pure white (no tint).
struct AdtChunkVertexColors {
    float c_outer[81 * 3];
    float c_inner[64 * 3];
    bool  parsed = false;
};

// One ADT texture layer inside a chunk. The MCLY sub-chunk gives 16 bytes
// per layer: { texture_index, flags, ofs_mcal, effect_id }. We capture all
// four. effect_id is a foreign key into GroundEffectTexture.dbc and drives
// the per-MCNK detail-grass scatter (the "ground cover" doodad system).
//
// alpha[] is the layer's 64x64 alpha map decoded out of MCAL. Per WoW's
// blending convention, layer 0 has no alpha map (it's the base, full
// coverage), and layers 1..n-1 cover 0..1. The mesh builder packs the
// three "upper" layers into the R/G/B channels of a 64x64 RGBA slice.
struct AdtLayer {
    int      texture_index = -1;   // -1 = no layer
    uint32_t flags = 0;
    uint32_t effect_id = 0;        // GroundEffectTexture.dbc row id (0 = none)
    uint8_t  alpha[64 * 64] = {};
};

// One MCNK chunk's parsed data.
struct AdtChunk {
    // World-space position of the chunk's NW corner. We store this as the
    // raw WoW coords (X south, Y east, Z up). Conversion to engine-space
    // happens at mesh-build time.
    float wow_x = 0.0f;
    float wow_y = 0.0f;
    float wow_z_base = 0.0f;

    AdtChunkHeights heights{};
    AdtChunkNormals normals{};
    AdtChunkVertexColors vertex_colors{};

    // Up to 4 layers per chunk. Layers beyond layer_count have default
    // (-1) texture_index and zero-filled alpha.
    AdtLayer layers[4]{};
    int layer_count = 0;

    // 16-bit mask describing which of the 4x4 sub-cells are "holes" (no
    // terrain - typically caves or cliff overhangs). Captured here for
    // completeness; the R2 mesh builder ignores it. R3 honors holes by
    // skipping the affected triangles.
    uint16_t holes_low_res = 0;
};

// One MDDF entry. Each ADT tile lists 0..N of these to place static
// M2 doodads (trees, rocks, fences, lanterns) at specific world
// positions. We keep the raw WoW coordinates and Euler degrees rather
// than pre-converting; the spawn code does WoW->engine + degrees->quat
// at the call site so the parser stays close to the on-disk layout.
//
// On-disk format (36 bytes per entry):
//   uint32 name_id    - index into AdtTile::doodad_paths
//   uint32 unique_id  - globally unique across the WoW map; used to
//                       dedupe across tiles when neighbors list the
//                       same prop on a shared border
//   float  pos[3]     - (X south, Y up, Z east) in WoW coords
//   float  rot[3]     - Euler degrees, applied YXZ (yaw, pitch, roll)
//   uint16 scale      - fixed point, 1024 == 1.0
//   uint16 flags      - bit 0 = biodome, bit 1 = shrubbery, etc.
//                       (unused for v1)
struct AdtDoodadPlacement {
    uint32_t name_id      = 0;
    uint32_t unique_id    = 0;
    float    pos_x        = 0.0f;   // WoW south axis
    float    pos_y        = 0.0f;   // WoW UP axis (height)
    float    pos_z        = 0.0f;   // WoW east axis
    float    rot_x_deg    = 0.0f;   // pitch (rotation about WoW X)
    float    rot_y_deg    = 0.0f;   // yaw   (rotation about WoW Y, up)
    float    rot_z_deg    = 0.0f;   // roll  (rotation about WoW Z)
    float    scale        = 1.0f;   // scale / 1024.0, clamped to [0.01, 100]
    uint16_t flags        = 0;
};

// Parsed ADT terrain content.
struct AdtTile {
    int tile_x = 0;             // 0..63 along WoW X axis
    int tile_y = 0;             // 0..63 along WoW Y axis

    // Texture filenames referenced by chunks (null-terminated strings from
    // MTEX). For R1 we don't load them yet but the loader still extracts
    // the list so the rendering branch downstream has them ready.
    std::vector<std::string> textures;

    // M2 doodad paths resolved from MMDX (string blob) via MMID (byte
    // offsets). doodad_paths[i] is the path for any MDDF entry whose
    // name_id == i. Backslashed WoW-style paths; the spawn code maps
    // them through ResolveWowAsset.
    std::vector<std::string> doodad_paths;

    // Parsed MDDF placements. Typical Elwynn tile has 100-500 entries.
    std::vector<AdtDoodadPlacement> doodads;

    // 256 MCNK chunks, indexed as [y * 16 + x] where x and y are the chunk
    // coordinates within the tile (0..15).
    AdtChunk chunks[kAdtChunksPerTile];
};

class AdtLoader {
public:
    // Parses an ADT file from disk. R1 parses only the chunks needed for
    // a flat heightmap render (MHDR, 256 MCNK each with MCVT). MTEX and
    // MCLY are also captured for future texture work but textures aren't
    // loaded yet.
    static bool LoadFile(const std::string& path, AdtTile& out);
};

} // namespace mve

#endif // MVE_ADT_TYPES_H
