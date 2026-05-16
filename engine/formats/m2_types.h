#ifndef MVE_M2_TYPES_H
#define MVE_M2_TYPES_H

#include <cstdint>

#pragma pack(push, 1)

namespace mve::m2 {

// Generic offset+count pair used throughout M2 files
template<typename T = void>
struct M2Array {
    uint32_t count;
    uint32_t offset; // byte offset from start of file
};

struct C3Vector { float x, y, z; };
struct C2Vector { float x, y; };
struct C4Vector { float x, y, z, w; };
struct C3sVector { int16_t x, y, z; }; // compressed short vector

// The M2 file header (WotLK version = 264)
struct M2Header {
    uint32_t magic;                    // 0x000  "MD20"
    uint32_t version;                  // 0x004  264 for WotLK

    M2Array<char> name;                // 0x008  model name string

    uint32_t global_flags;             // 0x010

    M2Array<> global_sequences;        // 0x014
    M2Array<> animations;              // 0x01C
    M2Array<> animation_lookup;        // 0x024

    M2Array<> bones;                   // 0x02C
    M2Array<> key_bone_lookup;         // 0x034

    M2Array<> vertices;                // 0x03C

    uint32_t num_skin_profiles;        // 0x044  (WotLK: skin files are separate .skin files)

    M2Array<> colors;                  // 0x048
    M2Array<> textures;                // 0x050
    M2Array<> transparency;            // 0x058
    M2Array<> texture_animations;      // 0x060
    M2Array<> texture_replace;         // 0x068

    M2Array<> render_flags;            // 0x070
    M2Array<> bone_lookup;             // 0x078
    M2Array<> texture_lookup;          // 0x080
    M2Array<> texture_units;           // 0x088
    M2Array<> transparency_lookup;     // 0x090
    M2Array<> texture_animation_lookup;// 0x098

    // Bounding box
    C3Vector bounding_box_min;         // 0x0A0
    C3Vector bounding_box_max;         // 0x0AC
    float bounding_sphere_radius;      // 0x0B8

    // Collision geometry
    C3Vector collision_box_min;        // 0x0BC
    C3Vector collision_box_max;        // 0x0C8
    float collision_sphere_radius;     // 0x0D4

    M2Array<> collision_triangles;     // 0x0D8
    M2Array<> collision_vertices;      // 0x0E0
    M2Array<> collision_normals;       // 0x0E8

    M2Array<> attachments;             // 0x0F0
    M2Array<> attachment_lookup;       // 0x0F8
    M2Array<> events;                  // 0x100
    M2Array<> lights;                  // 0x108
    M2Array<> cameras;                 // 0x110
    M2Array<> camera_lookup;           // 0x118
    M2Array<> ribbon_emitters;         // 0x120
    M2Array<> particle_emitters;       // 0x128
};

static_assert(sizeof(M2Header) == 304, "M2Header must be 304 bytes (0x130)");

// M2 vertex - 48 bytes per vertex
struct M2Vertex {
    C3Vector position;         // 3 floats = 12 bytes
    uint8_t  bone_weights[4];  // 4 bytes, values 0-255, sum should be 255
    uint8_t  bone_indices[4];  // 4 bytes, indices into bone lookup table
    C3Vector normal;           // 3 floats = 12 bytes
    C2Vector tex_coords[2];   // 2 UV sets, 2 floats each = 16 bytes
};

static_assert(sizeof(M2Vertex) == 48, "M2Vertex must be 48 bytes");

// M2 texture definition - references a BLP texture file
struct M2Texture {
    uint32_t type;             // 0 = file texture, others = procedural
    uint32_t flags;            // 1 = wrap_x, 2 = wrap_y
    M2Array<char> filename;    // path to BLP file (e.g., "Textures\\Creature\\Bear\\BearSkin.blp")
};

// M2 render flags - controls blend mode, two-sided, depth, etc.
struct M2Material {
    uint16_t flags;            // 1=unlit, 2=unfogged, 4=two_sided, 8=depth_test, 16=depth_write
    uint16_t blend_mode;       // 0=opaque, 1=alpha_key, 2=alpha, 3=no_alpha_add, 4=add, 5=mod, 6=mod2x
};

// M2 bone (M2CompBone)
struct M2CompBone {
    int32_t  key_bone_id;      // -1 if not a key bone
    uint32_t flags;
    int16_t  parent_bone;      // -1 for root bones
    uint16_t submesh_id;

    uint16_t bone_name_crc;    // or uDistToFurthDesc
    uint16_t bone_name_crc2;   // or uZRatioOfChain

    // Animation tracks (M2Track structures)
    // Each M2Track: interpolation_type(u16), global_sequence(s16), timestamps(M2Array), values(M2Array)
    struct {
        uint16_t interpolation_type;
        int16_t  global_sequence;
        M2Array<> timestamps;
        M2Array<> values;
    } translation, rotation, scaling;

    C3Vector pivot;            // the bone's pivot point in model space
};

static_assert(sizeof(M2CompBone) == 88, "M2CompBone must be 88 bytes");

// M2 animation sequence definition
struct M2Sequence {
    uint16_t id;               // animation ID (maps to AnimationData.dbc)
    uint16_t variation_index;
    uint32_t duration;         // in milliseconds
    float    movespeed;
    uint32_t flags;
    int16_t  frequency;
    uint16_t padding;
    uint32_t replay_min;
    uint32_t replay_max;
    uint32_t blend_time;
    C3Vector extent_min;
    C3Vector extent_max;
    float    extent_radius;
    int16_t  next_animation;
    uint16_t alias_next;
};

// --- Skin file structures ---

static constexpr uint32_t SKIN_MAGIC = 0x4E494B53; // "SKIN"

struct M2SkinHeader {
    uint32_t magic;                    // "SKIN"
    M2Array<uint16_t> vertices;        // local vertex indices → M2 global vertices
    M2Array<uint16_t> indices;         // triangle indices into the local vertex list
    M2Array<uint32_t> bones;           // bone combination indices
    M2Array<> submeshes;               // M2SkinSection entries
    M2Array<> batches;                 // M2Batch (texture unit) entries
    uint32_t bone_count_max;
};

struct M2SkinSection {
    uint16_t skin_section_id;
    uint16_t level;                    // LOD level
    uint16_t vertex_start;             // start in skin's local vertex list
    uint16_t vertex_count;
    uint16_t index_start;              // start in skin's index list
    uint16_t index_count;
    uint16_t bone_count;
    uint16_t bone_combo_index;         // into bone combination table
    uint16_t bone_influences;          // max bones per vertex in this submesh
    uint16_t center_bone_index;
    C3Vector center_position;
    C3Vector sort_center_position;
    float    sort_radius;
};

struct M2Batch {
    uint8_t  flags;
    int8_t   priority_plane;
    int16_t  shader_id;
    uint16_t skin_section_index;
    uint16_t geoset_index;
    int16_t  color_index;
    uint16_t material_index;          // index into M2Material (render_flags)
    uint16_t material_layer;
    uint16_t texture_count;
    uint16_t texture_combo_index;     // index into texture lookup
    uint16_t texture_coord_combo_index;
    uint16_t texture_weight_combo_index;
    uint16_t texture_transform_combo_index;
};

} // namespace mve::m2

#pragma pack(pop)

#endif // MVE_M2_TYPES_H
