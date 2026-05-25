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

// One MODF entry. Each ADT tile lists 0..N of these to place WMO
// buildings (the Abbey, Stormwind walls, Lion's Pride Inn, gnoll tents,
// etc.) at specific world positions. Same on-disk pos/rot/scale
// convention as MDDF, but the entry is 64 bytes instead of 36 - the
// extra fields are an axis-aligned bounding box, the doodad_set
// selector (each WMO ships with up to N "doodad sets" of interior
// props; this picks which set is active), and a name_set.
//
// On-disk format (64 bytes per entry):
//   uint32 name_id      - index into AdtTile::wmo_paths
//   uint32 unique_id    - global dedup key (same convention as MDDF)
//   float  pos[3]       - (X south, Y up, Z east) in WoW coords
//   float  rot[3]       - Euler degrees, applied YXZ
//   float  bbox_lo[3]   - AABB in world-space, used for visibility cull
//   float  bbox_hi[3]
//   uint16 flags        - bit 0x1 = "destructible", others ignored for v1
//   uint16 doodad_set   - selects which MODS (doodad set) is active
//   uint16 name_set     - selects which set of overlay textures
//   uint16 scale        - WoW pre-Cata: ignored, scale always 1.0
struct AdtWmoPlacement {
    uint32_t name_id    = 0;
    uint32_t unique_id  = 0;
    float    pos_x      = 0.0f;
    float    pos_y      = 0.0f;
    float    pos_z      = 0.0f;
    float    rot_x_deg  = 0.0f;
    float    rot_y_deg  = 0.0f;
    float    rot_z_deg  = 0.0f;
    float    bbox_lo[3] = {0.0f, 0.0f, 0.0f};
    float    bbox_hi[3] = {0.0f, 0.0f, 0.0f};
    uint16_t flags      = 0;
    uint16_t doodad_set = 0;
    uint16_t name_set   = 0;
    uint16_t scale      = 1024;
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

// One liquid (water/ocean/magma/slime) instance parsed from a single
// SMLiquidInstance inside the top-level MH2O chunk. WotLK 3.3.5a uses
// MH2O exclusively; the legacy per-MCNK MCLQ format is not parsed here.
//
// Coordinate frame: positions are computed at mesh-build time from
// (chunk_index, x_offset, y_offset). The chunk_index references which
// MCNK (0..255, row-major iy*16+ix) the liquid is anchored to; the
// offsets and dimensions then carve out a (width x height) cell
// rectangle inside that MCNK, where each cell is kAdtChunkSize / 8 =
// 4.1666... yards on a side. So the patch spans width*cell_size by
// height*cell_size yards starting at the MCNK's NW + offset cells.
//
// LVF (LiquidVertexFormat) selected per liquid_object_or_lvf in the
// on-disk SMLiquidInstance (when <= 41, it IS the LVF directly):
//   0 = Height + Depth   (Elwynn rivers, lakes - standard)
//   1 = Height + UV      (magma/slime with painted UVs)
//   2 = Depth only       (ocean - flat, height = min_height_level)
//   3 = Height + UV + Depth
// Vertex count is always (width+1) * (height+1) regardless of LVF.
struct AdtLiquidInstance {
    uint16_t chunk_index = 0;        // 0..255, iy*16+ix into AdtTile::chunks
    uint16_t liquid_type = 0;        // FK -> LiquidType.dbc; 1=Slow Water (Elwynn)
    uint16_t lvf         = 0;        // 0..3 per LVF table above
    uint8_t  x_offset    = 0;        // 0..7 cells from MCNK NW
    uint8_t  y_offset    = 0;
    uint8_t  width       = 0;        // 1..8 cells wide
    uint8_t  height      = 0;        // 1..8 cells tall
    float    min_height  = 0.0f;     // WoW Z min/max (used for LVF=2 flat ocean)
    float    max_height  = 0.0f;
    // Per-vertex height in WoW absolute Z. (width+1)*(height+1) entries
    // row-major. For LVF=2 (ocean), populated to min_height for all verts.
    std::vector<float> heights;
    // Per-vertex depth in [0,1]. Drives the shallow/deep alpha blend.
    // Same packing as heights. For LVF=1 (no depth on disk), all 1.0.
    std::vector<float> depths;
    // Existence bitmap: bit (y*width + x) set means the cell at local
    // (x, y) is rendered. Empty vector = "all cells exist" (the wiki's
    // convention when offset_exists_bitmap == 0 in the on-disk struct).
    std::vector<uint8_t> exists_bits;
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

    // WMO (building) paths resolved from MWMO via MWID byte offsets.
    // wmo_paths[i] is the .wmo path for any MODF entry whose name_id
    // == i. Same backslashed-WoW-path convention as doodad_paths.
    std::vector<std::string> wmo_paths;

    // Parsed MODF placements. Typical Elwynn tile has 0-10 entries
    // (buildings are large and sparse vs. the dense MDDF prop list).
    std::vector<AdtWmoPlacement> wmos;

    // 256 MCNK chunks, indexed as [y * 16 + x] where x and y are the chunk
    // coordinates within the tile (0..15).
    AdtChunk chunks[kAdtChunksPerTile];

    // Parsed MH2O liquid instances (water/ocean/magma/slime patches).
    // Typical Elwynn tile: 0-30 instances mostly along Crystal Brook
    // and the Stormwind moat. Empty for tiles with no water.
    std::vector<AdtLiquidInstance> liquids;
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
