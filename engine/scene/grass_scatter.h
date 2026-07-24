#ifndef MVE_GRASS_SCATTER_H
#define MVE_GRASS_SCATTER_H

#include "../formats/adt_types.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mve {

// One GroundEffectTexture.dbc row. Schema (44 bytes, 11 uint32 fields):
//   Id, Doodad[4], DoodadWeight[4], Density, Sound
//
// Each terrain texture variation that participates in the ground-cover
// system points at one of these rows. Up to 4 doodad ids may be
// scattered for the layer, picked weighted-randomly per density cell
// from the (doodad_ids, doodad_weights) tables.
struct GroundEffectTextureRow {
    uint32_t id = 0;
    uint32_t doodad_ids[4]     = {0, 0, 0, 0};
    uint32_t doodad_weights[4] = {0, 0, 0, 0};
    uint32_t density = 0;
    uint32_t sound = 0;
};

// One GroundEffectDoodad.dbc row. Schema (12 bytes, 3 uint32 fields):
//   Id, DoodadPath (string offset), Flags
//
// `path` is the WoW-relative M2 path (resolved out of the DBC string
// block at load time). `flags` is a small bitfield (bit 0 = random
// rotation per instance, etc.); we honor random Y rotation always
// since it's the dominant grass visual feature.
struct GroundEffectDoodadRow {
    uint32_t id = 0;
    std::string path;
    uint32_t flags = 0;
};

// In-memory lookup tables loaded once from
//   assets/dbc/GroundEffectTexture.dbc
//   assets/dbc/GroundEffectDoodad.dbc
//
// Both are keyed by row id (the foreign key MCLY effect_id values
// reference). Missing ids resolve to nullptr - the scatter code skips
// the cell silently in that case.
class GroundEffectTables {
public:
    bool Load(const std::string& texture_dbc_path,
              const std::string& doodad_dbc_path);

    const GroundEffectTextureRow* GetTexture(uint32_t id) const;
    const GroundEffectDoodadRow*  GetDoodad(uint32_t id) const;

    size_t TextureCount() const { return pm_textures.size(); }
    size_t DoodadCount()  const { return pm_doodads.size(); }

private:
    std::unordered_map<uint32_t, GroundEffectTextureRow> pm_textures;
    std::unordered_map<uint32_t, GroundEffectDoodadRow>  pm_doodads;
};

// One scattered grass placement. The caller pushes these into the
// AssetManager's detail-grass queue exactly like an MDDF doodad, and
// the existing FlushDoodadInstances path turns them into a per-M2-path
// instanced entity.
//
// model_matrix is already in engine space (T * R * S applied). The
// scatter algorithm uses the chunk's MCVT to derive the ground height
// and a deterministic per-cell hash for the random Y rotation +
// scale jitter.
struct GrassPlacement {
    std::string wow_m2_path;       // backslashed WoW path, e.g. WORLD\AZEROTH\...
    glm::mat4   model_matrix;
};

// Compute grass placements for one ADT tile. Walks every MCNK, samples
// the layer-alpha mix per 8x8 sub-cell, looks up each layer's effect_id
// in the GroundEffect tables, and scatters Density * weight doodads
// per cell. Output is appended to `out`.
//
// The scatter is deterministic: it hashes (tile_xy, chunk_xy, sub_cell_xy,
// doodad_index) so reloading a tile produces identical placements (no
// "shimmer" if the same tile streams in twice).
//
// Density caps:
//   max_per_subcell - hard cap on instances per (subcell, layer) to
//   keep total counts manageable. 1 is a sensible v1 default; the WoW
//   client uses up to 4 but our depth-write-on alpha-keyed grass means
//   overdraw is the costly axis.
//
// The math:
//   Each MCNK is 33.33 yards. The 8x8 sub-cell grid means each sub-cell
//   is ~4.17 yards. We sample the chunk's alpha-map (already 64x64,
//   parsed into layers[].alpha[]) at the sub-cell center (texel 4 + i*8)
//   to get per-layer fractions. Density (DBC field) is "doodads per
//   unit area"; we clamp by max_per_subcell and weight by the layer
//   alpha so a 50%-covered layer gets half as many.
void ScatterGrassForTile(const AdtTile& tile,
                          const GroundEffectTables& tables,
                          int max_per_subcell,
                          std::vector<GrassPlacement>& out);

} // namespace mve

#endif // MVE_GRASS_SCATTER_H
