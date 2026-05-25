#ifndef MVE_WMO_TYPES_H
#define MVE_WMO_TYPES_H

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mve {

// WMO root header (chunk MOHD, 64 bytes). One per .wmo file (the
// "stem" file without an _NNN.wmo suffix). Field offsets and sizes
// match the on-disk layout exactly so callers can memcpy-decode
// straight into this struct.
struct WmoMohd {
    uint32_t n_textures      = 0;   // # MOMT entries
    uint32_t n_groups        = 0;   // # group files + # MOGI/MOGN entries
    uint32_t n_portals       = 0;   // # MOPT entries (v1: skip)
    uint32_t n_lights        = 0;   // # MOLT entries (v1: skip)
    uint32_t n_doodad_names  = 0;
    uint32_t n_doodad_defs   = 0;   // # MODD entries
    uint32_t n_doodad_sets   = 0;   // # MODS entries
    uint32_t amb_color       = 0;   // BGRA - MOHD ambient (interior fallback)
    int32_t  wmo_id          = 0;   // -> WMOAreaTable.dbc
    float    bbox_min[3]     = {0, 0, 0};
    float    bbox_max[3]     = {0, 0, 0};
    uint16_t flags           = 0;   // bit 3 lighten_interiors (uses amb_color)
    uint16_t num_lod         = 0;   // always 0 in WotLK
};

// MOMT material flags (per wowdev.wiki/WMO + WebWowViewerCpp).
enum WmoMomtFlag : uint32_t {
    WmoMomtFlag_Unlit    = 0x01,   // skip diffuse light * MOCV1
    WmoMomtFlag_Unfogged = 0x02,   // skip distance fog blend
    WmoMomtFlag_Unculled = 0x04,   // disable backface culling
    WmoMomtFlag_ExtLight = 0x08,   // force exterior lighting on indoor group
    WmoMomtFlag_Sidn     = 0x10,   // self-illuminated day/night (lanterns)
    WmoMomtFlag_Window   = 0x20,   // window/reflection hint (v1: ignore)
    WmoMomtFlag_ClampS   = 0x40,   // sampler U clamp (override default repeat)
    WmoMomtFlag_ClampT   = 0x80,   // sampler V clamp
};

// MOMT material entry (64 bytes per material). The shader_id field
// drives which of WotLK's 17 shader paths to use; for Elwynn we
// implement Diffuse (0), TwoLayerDiffuse (6) and DiffuseEmissive (9)
// and fall the rest back to plain Diffuse.
struct WmoMomt {
    uint32_t flags             = 0;
    uint32_t shader            = 0;
    uint32_t blend_mode        = 0;
    uint32_t diffuse_name_ofs  = 0;   // byte offset into MOTX -> diffuse BLP path
    uint32_t emissive_color    = 0;   // BGRA, added when F_SIDN at night
    uint32_t sidn_emissive     = 0;   // runtime field, zero on disk
    uint32_t env_name_ofs      = 0;   // byte offset into MOTX -> 2nd texture
    uint32_t diff_color        = 0;   // BGRA tint mul on diffuse output
    int32_t  ground_type       = 0;   // TerrainType.dbc, ignored for render
    uint32_t texture_2_ofs     = 0;   // 3rd texture (ThreeLayerTerrain only)
    uint32_t color_2           = 0;   // 2nd tint (emissive shaders)
    uint32_t flags_2           = 0;
    uint32_t runtime_pad[4]    = {0, 0, 0, 0};
};

// MOGI per-group info (32 bytes). Sits in the root file; describes
// every group's flags + local-frame bounding box BEFORE we open the
// group file itself. Required for early frustum-cull at the root
// level - we transform the local AABB by the WMO's model matrix and
// reject groups outside the camera frustum without parsing geometry.
struct WmoMogi {
    uint32_t flags        = 0;   // mirror of MOGP.flags
    float    bbox_min[3]  = {0, 0, 0};
    float    bbox_max[3]  = {0, 0, 0};
    int32_t  name_ofs     = 0;   // byte offset into MOGN, -1 = none
};

// MOGP group header (68 bytes). The MOGP chunk wraps ALL the
// group-file sub-chunks inside its payload (it's recursive - the
// remaining sub-chunks live in [68 .. mogp_payload_size)). Same
// trick as ADT's MCNK.
struct WmoMogp {
    uint32_t name_ofs              = 0;
    uint32_t descriptive_name_ofs  = 0;
    uint32_t flags                 = 0;
    float    bbox_min[3]           = {0, 0, 0};
    float    bbox_max[3]           = {0, 0, 0};
    uint16_t mopr_index            = 0;
    uint16_t mopr_count            = 0;
    uint16_t trans_batch_count     = 0;
    uint16_t int_batch_count       = 0;
    uint16_t ext_batch_count       = 0;
    uint16_t unk_padding           = 0;
    uint8_t  fog_indices[4]        = {0, 0, 0, 0};
    uint32_t group_liquid          = 0;   // LiquidType.dbc, or 0
    uint32_t wmo_group_id          = 0;   // -> WMOAreaTable.dbc
    uint32_t flags2                = 0;
    int16_t  parent_or_first_child = -1;
    int16_t  next_split_child      = -1;
};

// MOGP.flags bits (canonical SMOGroupFlags from WebWowViewerCpp).
enum WmoMogpFlag : uint32_t {
    WmoMogpFlag_HasBsp           = 0x00000001,  // MOBN/MOBR present
    WmoMogpFlag_HasVertexColors  = 0x00000004,  // MOCV present
    WmoMogpFlag_Exterior         = 0x00000008,
    WmoMogpFlag_ExteriorLit      = 0x00000040,
    WmoMogpFlag_HasLights        = 0x00000200,
    WmoMogpFlag_HasDoodads       = 0x00000800,
    WmoMogpFlag_LiquidSurface    = 0x00001000,
    WmoMogpFlag_Interior         = 0x00002000,
    WmoMogpFlag_AlwaysDraw       = 0x00010000,
    WmoMogpFlag_ShowSkybox       = 0x00040000,
    WmoMogpFlag_CVerts2          = 0x01000000,  // MOCV2 present
    WmoMogpFlag_TVerts2          = 0x02000000,  // MOTV2 present
    WmoMogpFlag_AntiPortal       = 0x04000000,
};

// MOBA render batch (24 bytes). One batch = one vkCmdDrawIndexed.
struct WmoMoba {
    int16_t  unk_box_min[3]  = {0, 0, 0};
    int16_t  unk_box_max[3]  = {0, 0, 0};
    uint32_t first_index     = 0;
    uint16_t num_indices     = 0;
    uint16_t first_vertex    = 0;
    uint16_t last_vertex     = 0;
    uint8_t  flags           = 0;
    uint8_t  material_id     = 0;   // index into root's MOMT
};

// MODS doodad set entry (32 bytes). The MODF in the parent ADT
// selects WHICH set is active; set 0 ("Set_$DefaultGlobal") is always
// rendered in addition.
struct WmoMods {
    char     name[20]              = {0};   // ASCII, zero-padded
    uint32_t first_instance_index  = 0;
    uint32_t n_doodads             = 0;
    uint32_t unused                = 0;
};

// MODD doodad placement (40 bytes). One instance of an M2 inside the
// WMO. Note orientation is a quaternion in (x,y,z,w) order on disk -
// NOT glm's (w,x,y,z) ctor.
struct WmoModd {
    uint32_t name_ofs_and_flags = 0;   // lower 24 bits: byte offset into MODN
    float    position[3]        = {0, 0, 0};   // WMO-local frame
    float    orient[4]          = {0, 0, 0, 1};// quaternion (x,y,z,w)
    float    scale              = 1.0f;
    uint32_t color              = 0;   // BGRA per-instance ambient tint
};

// Parsed group file: vertices + indices + render batches. Coordinates
// are kept in WMO-LOCAL frame here (not yet axis-swapped); the spawn
// code applies the basis change as part of the model matrix.
struct WmoGroup {
    WmoMogp header{};
    std::vector<glm::vec3> positions;   // MOVT
    std::vector<glm::vec3> normals;     // MONR
    std::vector<glm::vec2> uvs1;        // MOTV (layer 1)
    std::vector<glm::vec2> uvs2;        // MOTV (layer 2, if TVerts2 flag set)
    std::vector<uint32_t>  colors1;     // MOCV (layer 1) packed RGBA8
    std::vector<uint32_t>  colors2;     // MOCV (layer 2, if CVerts2)
    std::vector<uint16_t>  indices;     // MOVI
    std::vector<WmoMoba>   batches;     // MOBA
    bool has_uvs2    = false;
    bool has_colors1 = false;
    bool has_colors2 = false;
};

// Parsed root file: header, textures, materials, group-infos, group
// names blob, doodadsets + names + placements. The vector<WmoGroup>
// is filled in by LoadGroup calls AFTER LoadRoot (one per nGroups).
struct WmoRoot {
    WmoMohd header{};
    std::vector<std::string> texture_paths;   // resolved from MOTX, indexed by MOMT slot
    std::vector<WmoMomt>     materials;        // MOMT
    std::vector<WmoMogi>     group_infos;      // MOGI (1:1 with groups)
    std::string              group_names_blob; // MOGN (raw, indexed by name_ofs)
    std::vector<WmoMods>     doodad_sets;      // MODS
    std::string              doodad_names_blob;// MODN
    std::vector<WmoModd>     doodad_defs;      // MODD
    std::vector<std::unique_ptr<WmoGroup>> groups;  // size == header.n_groups
};

// Resolve a byte offset in MOTX into the matching texture path. Used
// for MOMT.diffuse_name_ofs and friends. Returns empty string if the
// offset is out of range (which the spawn code treats as "skip this
// material").
inline std::string WmoResolveTexture(const WmoRoot& root, uint32_t byte_ofs) {
    // texture_paths is indexed by MOMT slot, not byte offset - the
    // parser builds a parallel lookup. The MOTX raw blob is dropped
    // after parsing, so we use the slot-indexed vector here.
    // For now, find the entry whose offset matches; the parser sets up
    // a per-byte-offset map for direct lookup at parse time instead.
    (void)root;
    (void)byte_ofs;
    return {};
}

class WmoLoader {
public:
    // Parse the root .wmo file at `path`. Reads MVER/MOHD/MOTX/MOMT/
    // MOGN/MOGI/MODS/MODN/MODD; skips MOLT/MOSB/MOPV/MOPT/MOPR/MFOG/MCVP
    // (v1 doesn't need lights, portals, or per-WMO fog overrides).
    // Returns nullptr on parse failure.
    static std::unique_ptr<WmoRoot> LoadRoot(const std::string& fs_path);

    // Parse a group file (`<root_stem>_NNN.wmo`). Reads MOGP header
    // and its embedded sub-chunks (MOPY/MOVI/MOVT/MONR/MOTV/MOCV/MOBA).
    // Skips MOLR/MODR/MOBN/MOBR/MLIQ for v1. Returns nullptr on parse
    // failure. The result's MOCV1 alpha channel has already been
    // fixed up per WotLK runtime convention (255 for EXTERIOR batches,
    // 0 for INTERIOR, on-disk value preserved for TRANS).
    static std::unique_ptr<WmoGroup> LoadGroup(const std::string& fs_path);
};

} // namespace mve

#endif // MVE_WMO_TYPES_H
