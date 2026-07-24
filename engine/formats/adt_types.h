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

// One MCNK chunk's parsed data we care about for R1 (terrain only).
struct AdtChunk {
    // World-space position of the chunk's NW corner. We store this as the
    // raw WoW coords (X south, Y east, Z up). Conversion to engine-space
    // happens at mesh-build time.
    float wow_x = 0.0f;
    float wow_y = 0.0f;
    float wow_z_base = 0.0f;

    AdtChunkHeights heights{};

    // Per-chunk texture layer indices into the parent ADT's texture list.
    // Up to 4 layers. -1 = no layer. R1 keeps these for later texturing.
    int layer_texture[4] = { -1, -1, -1, -1 };
    int layer_count = 0;
};

// Parsed ADT terrain content.
struct AdtTile {
    int tile_x = 0;             // 0..63 along WoW X axis
    int tile_y = 0;             // 0..63 along WoW Y axis

    // Texture filenames referenced by chunks (null-terminated strings from
    // MTEX). For R1 we don't load them yet but the loader still extracts
    // the list so the rendering branch downstream has them ready.
    std::vector<std::string> textures;

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
