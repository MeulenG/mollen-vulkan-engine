#include "wmo_types.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace mve {

namespace {

// FourCC builder identical to adt_loader's - WoW chunk IDs appear on
// disk in REVERSE byte order, so a literal "MVER" needs to be packed
// with 'R' in the LSB. Duplicated here to avoid coupling the two
// loaders through a shared header.
constexpr uint32_t FourCC(const char* s) {
    return (uint32_t)(uint8_t)s[3]
         | ((uint32_t)(uint8_t)s[2] << 8)
         | ((uint32_t)(uint8_t)s[1] << 16)
         | ((uint32_t)(uint8_t)s[0] << 24);
}

inline uint32_t ReadU32(const uint8_t* p, size_t off) {
    uint32_t v = 0;
    std::memcpy(&v, p + off, 4);
    return v;
}
inline uint16_t ReadU16(const uint8_t* p, size_t off) {
    uint16_t v = 0;
    std::memcpy(&v, p + off, 2);
    return v;
}
inline int16_t ReadI16(const uint8_t* p, size_t off) {
    int16_t v = 0;
    std::memcpy(&v, p + off, 2);
    return v;
}
inline float ReadF32(const uint8_t* p, size_t off) {
    float v = 0;
    std::memcpy(&v, p + off, 4);
    return v;
}

bool ReadFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;
    in.seekg(0, std::ios::end);
    std::streamsize sz = in.tellg();
    in.seekg(0, std::ios::beg);
    if (sz <= 0) return false;
    out.resize(static_cast<size_t>(sz));
    return static_cast<bool>(in.read(reinterpret_cast<char*>(out.data()), sz));
}

// Resolve a null-terminated string starting at `ofs` inside `blob`.
// Defends against missing null with strnlen.
std::string ReadCString(const std::string& blob, uint32_t ofs) {
    if (ofs >= blob.size()) return {};
    const char* s = blob.data() + ofs;
    size_t max_len = blob.size() - ofs;
    return std::string(s, ::strnlen(s, max_len));
}

} // namespace

std::unique_ptr<WmoRoot> WmoLoader::LoadRoot(const std::string& fs_path) {
    std::vector<uint8_t> buf;
    if (!ReadFile(fs_path, buf)) {
        std::fprintf(stderr, "WMO root open failed: %s\n", fs_path.c_str());
        return nullptr;
    }

    auto root = std::make_unique<WmoRoot>();
    std::string motx_blob;   // raw texture-name blob, indexed by byte offset

    size_t pos = 0;
    while (pos + 8 <= buf.size()) {
        uint32_t id   = ReadU32(buf.data(), pos);
        uint32_t size = ReadU32(buf.data(), pos + 4);
        pos += 8;
        if (pos + size > buf.size()) break;
        const uint8_t* data = buf.data() + pos;

        if (id == FourCC("MOHD") && size >= 64) {
            auto& h = root->header;
            h.n_textures     = ReadU32(data, 0);
            h.n_groups       = ReadU32(data, 4);
            h.n_portals      = ReadU32(data, 8);
            h.n_lights       = ReadU32(data, 12);
            h.n_doodad_names = ReadU32(data, 16);
            h.n_doodad_defs  = ReadU32(data, 20);
            h.n_doodad_sets  = ReadU32(data, 24);
            h.amb_color      = ReadU32(data, 28);
            h.wmo_id         = static_cast<int32_t>(ReadU32(data, 32));
            for (int i = 0; i < 3; ++i) h.bbox_min[i] = ReadF32(data, 36 + i * 4);
            for (int i = 0; i < 3; ++i) h.bbox_max[i] = ReadF32(data, 48 + i * 4);
            h.flags   = ReadU16(data, 60);
            h.num_lod = ReadU16(data, 62);
        } else if (id == FourCC("MOTX")) {
            motx_blob.assign(reinterpret_cast<const char*>(data), size);
        } else if (id == FourCC("MOMT")) {
            uint32_t count = size / 64u;
            root->materials.reserve(count);
            root->texture_paths.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                size_t mo = i * 64u;
                WmoMomt m{};
                m.flags            = ReadU32(data, mo + 0);
                m.shader           = ReadU32(data, mo + 4);
                m.blend_mode       = ReadU32(data, mo + 8);
                m.diffuse_name_ofs = ReadU32(data, mo + 12);
                m.emissive_color   = ReadU32(data, mo + 16);
                m.sidn_emissive    = ReadU32(data, mo + 20);
                m.env_name_ofs     = ReadU32(data, mo + 24);
                m.diff_color       = ReadU32(data, mo + 28);
                m.ground_type      = static_cast<int32_t>(ReadU32(data, mo + 32));
                m.texture_2_ofs    = ReadU32(data, mo + 36);
                m.color_2          = ReadU32(data, mo + 40);
                m.flags_2          = ReadU32(data, mo + 44);
                root->materials.push_back(m);
                // Stash the diffuse texture path for slot i. Other texture
                // slots (env, texture_2) resolve on demand at draw setup
                // for the few shaders that need them.
                root->texture_paths.push_back(
                    ReadCString(motx_blob, m.diffuse_name_ofs));
            }
        } else if (id == FourCC("MOGN")) {
            root->group_names_blob.assign(
                reinterpret_cast<const char*>(data), size);
        } else if (id == FourCC("MOGI")) {
            uint32_t count = size / 32u;
            root->group_infos.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                size_t io = i * 32u;
                WmoMogi g{};
                g.flags       = ReadU32(data, io + 0);
                for (int k = 0; k < 3; ++k) g.bbox_min[k] = ReadF32(data, io + 4 + k * 4);
                for (int k = 0; k < 3; ++k) g.bbox_max[k] = ReadF32(data, io + 16 + k * 4);
                g.name_ofs    = static_cast<int32_t>(ReadU32(data, io + 28));
                root->group_infos.push_back(g);
            }
        } else if (id == FourCC("MODS")) {
            uint32_t count = size / 32u;
            root->doodad_sets.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                size_t so = i * 32u;
                WmoMods s{};
                std::memcpy(s.name, data + so, 20);
                s.first_instance_index = ReadU32(data, so + 20);
                s.n_doodads            = ReadU32(data, so + 24);
                s.unused               = ReadU32(data, so + 28);
                root->doodad_sets.push_back(s);
            }
        } else if (id == FourCC("MODN")) {
            root->doodad_names_blob.assign(
                reinterpret_cast<const char*>(data), size);
        } else if (id == FourCC("MODD")) {
            uint32_t count = size / 40u;
            root->doodad_defs.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                size_t mo = i * 40u;
                WmoModd m{};
                m.name_ofs_and_flags = ReadU32(data, mo + 0);
                m.position[0] = ReadF32(data, mo + 4);
                m.position[1] = ReadF32(data, mo + 8);
                m.position[2] = ReadF32(data, mo + 12);
                m.orient[0]   = ReadF32(data, mo + 16);
                m.orient[1]   = ReadF32(data, mo + 20);
                m.orient[2]   = ReadF32(data, mo + 24);
                m.orient[3]   = ReadF32(data, mo + 28);
                m.scale       = ReadF32(data, mo + 32);
                m.color       = ReadU32(data, mo + 36);
                root->doodad_defs.push_back(m);
            }
        }
        // Skipped: MVER (version, always 17 for WotLK), MOSB (skybox
        // model path), MOPV/MOPT/MOPR (portals), MOLT (lights), MOVV/
        // MOVB (visibility blocks), MFOG (per-WMO fog), MCVP (clip
        // volumes). None required for the v1 textured render.

        pos += size;
    }

    root->groups.resize(root->header.n_groups);
    return root;
}

std::unique_ptr<WmoGroup> WmoLoader::LoadGroup(const std::string& fs_path) {
    std::vector<uint8_t> buf;
    if (!ReadFile(fs_path, buf)) {
        std::fprintf(stderr, "WMO group open failed: %s\n", fs_path.c_str());
        return nullptr;
    }

    auto grp = std::make_unique<WmoGroup>();

    // Walk top-level: MVER + MOGP. MOGP is recursive - its payload
    // contains all the geometry sub-chunks after a 76-byte header.
    size_t pos = 0;
    bool mogp_found = false;
    while (pos + 8 <= buf.size()) {
        uint32_t id   = ReadU32(buf.data(), pos);
        uint32_t size = ReadU32(buf.data(), pos + 4);
        pos += 8;
        if (pos + size > buf.size()) break;
        if (id != FourCC("MOGP")) { pos += size; continue; }
        mogp_found = true;

        const uint8_t* mogp = buf.data() + pos;
        if (size < 76) break;

        WmoMogp& h = grp->header;
        h.name_ofs              = ReadU32(mogp, 0);
        h.descriptive_name_ofs  = ReadU32(mogp, 4);
        h.flags                 = ReadU32(mogp, 8);
        for (int i = 0; i < 3; ++i) h.bbox_min[i] = ReadF32(mogp, 12 + i * 4);
        for (int i = 0; i < 3; ++i) h.bbox_max[i] = ReadF32(mogp, 24 + i * 4);
        h.mopr_index            = ReadU16(mogp, 36);
        h.mopr_count            = ReadU16(mogp, 38);
        h.trans_batch_count     = ReadU16(mogp, 40);
        h.int_batch_count       = ReadU16(mogp, 42);
        h.ext_batch_count       = ReadU16(mogp, 44);
        h.unk_padding           = ReadU16(mogp, 46);
        for (int i = 0; i < 4; ++i) h.fog_indices[i] = mogp[48 + i];
        h.group_liquid          = ReadU32(mogp, 52);
        h.wmo_group_id          = ReadU32(mogp, 56);
        h.flags2                = ReadU32(mogp, 60);
        h.parent_or_first_child = ReadI16(mogp, 64);
        h.next_split_child      = ReadI16(mogp, 66);

        // MOGP header is 68 bytes (verified by hex-dumping Stormwind_000.wmo:
        // MVER header at file offset 0, MOGP chunk header at 0x0C, MOGP
        // payload at 0x14, first sub-chunk MOPY at 0x58 = 0x14 + 68).
        // Sub-chunks live in [68 .. size). Recursive scan.
        size_t sub_pos = 68;
        const uint8_t* sub_base = mogp;
        size_t sub_end = size;
        while (sub_pos + 8 <= sub_end) {
            uint32_t sid = ReadU32(sub_base, sub_pos);
            uint32_t ssz = ReadU32(sub_base, sub_pos + 4);
            sub_pos += 8;
            if (sub_pos + ssz > sub_end) break;
            const uint8_t* sdata = sub_base + sub_pos;

            if (sid == FourCC("MOVI") && (ssz % 2u) == 0u) {
                uint32_t n = ssz / 2u;
                grp->indices.resize(n);
                for (uint32_t i = 0; i < n; ++i) {
                    grp->indices[i] = ReadU16(sdata, i * 2u);
                }
            } else if (sid == FourCC("MOVT") && (ssz % 12u) == 0u) {
                uint32_t n = ssz / 12u;
                grp->positions.resize(n);
                for (uint32_t i = 0; i < n; ++i) {
                    grp->positions[i].x = ReadF32(sdata, i * 12u + 0);
                    grp->positions[i].y = ReadF32(sdata, i * 12u + 4);
                    grp->positions[i].z = ReadF32(sdata, i * 12u + 8);
                }
            } else if (sid == FourCC("MONR") && (ssz % 12u) == 0u) {
                uint32_t n = ssz / 12u;
                grp->normals.resize(n);
                for (uint32_t i = 0; i < n; ++i) {
                    grp->normals[i].x = ReadF32(sdata, i * 12u + 0);
                    grp->normals[i].y = ReadF32(sdata, i * 12u + 4);
                    grp->normals[i].z = ReadF32(sdata, i * 12u + 8);
                }
            } else if (sid == FourCC("MOTV") && (ssz % 8u) == 0u) {
                uint32_t n = ssz / 8u;
                // First MOTV = uvs1; if TVerts2 flag is set, a second
                // MOTV follows.
                if (grp->uvs1.empty()) {
                    grp->uvs1.resize(n);
                    for (uint32_t i = 0; i < n; ++i) {
                        grp->uvs1[i].x = ReadF32(sdata, i * 8u + 0);
                        grp->uvs1[i].y = ReadF32(sdata, i * 8u + 4);
                    }
                } else {
                    grp->uvs2.resize(n);
                    for (uint32_t i = 0; i < n; ++i) {
                        grp->uvs2[i].x = ReadF32(sdata, i * 8u + 0);
                        grp->uvs2[i].y = ReadF32(sdata, i * 8u + 4);
                    }
                    grp->has_uvs2 = true;
                }
            } else if (sid == FourCC("MOCV") && (ssz % 4u) == 0u) {
                uint32_t n = ssz / 4u;
                if (!grp->has_colors1) {
                    grp->colors1.resize(n);
                    for (uint32_t i = 0; i < n; ++i) {
                        grp->colors1[i] = ReadU32(sdata, i * 4u);
                    }
                    grp->has_colors1 = true;
                } else {
                    grp->colors2.resize(n);
                    for (uint32_t i = 0; i < n; ++i) {
                        grp->colors2[i] = ReadU32(sdata, i * 4u);
                    }
                    grp->has_colors2 = true;
                }
            } else if (sid == FourCC("MOBA") && (ssz % 24u) == 0u) {
                uint32_t n = ssz / 24u;
                grp->batches.reserve(n);
                for (uint32_t i = 0; i < n; ++i) {
                    size_t bo = i * 24u;
                    WmoMoba b{};
                    for (int k = 0; k < 3; ++k) b.unk_box_min[k] = ReadI16(sdata, bo + k * 2);
                    for (int k = 0; k < 3; ++k) b.unk_box_max[k] = ReadI16(sdata, bo + 6 + k * 2);
                    b.first_index  = ReadU32(sdata, bo + 12);
                    b.num_indices  = ReadU16(sdata, bo + 16);
                    b.first_vertex = ReadU16(sdata, bo + 18);
                    b.last_vertex  = ReadU16(sdata, bo + 20);
                    b.flags        = sdata[bo + 22];
                    b.material_id  = sdata[bo + 23];
                    grp->batches.push_back(b);
                }
            }
            // Skipped: MOPY (per-triangle materials, only needed for
            // collision builds since MOBA already filters renderable
            // triangles), MOLR/MODR (light/doodad refs - we draw
            // doodads via the root-level MODD list regardless),
            // MOBN/MOBR (BSP collision), MLIQ (per-group liquid),
            // MOTA (tangents, Cata+ only).

            sub_pos += ssz;
        }
        break;  // only one MOGP per group file
    }

    if (!mogp_found) {
        std::fprintf(stderr, "WMO group missing MOGP: %s\n", fs_path.c_str());
        return nullptr;
    }

    // MOCV1 alpha runtime fixup (per wowdev.wiki/WMO/Rendering): the
    // on-disk alpha encodes interior/exterior blend for TRANS batches
    // only. For pure EXTERIOR / INTERIOR batches the client overwrites
    // alpha to 255/0 respectively before shading. Without this,
    // interiors render washed out.
    if (grp->has_colors1 && !grp->colors1.empty()) {
        uint16_t total = grp->header.trans_batch_count +
                         grp->header.int_batch_count +
                         grp->header.ext_batch_count;
        if (total == grp->batches.size()) {
            uint16_t trans_end = grp->header.trans_batch_count;
            uint16_t int_end   = trans_end + grp->header.int_batch_count;
            // Walk batches: trans keeps disk alpha, interior gets 0,
            // exterior gets 255. Mark each vertex via the batch's
            // first_vertex/last_vertex range.
            for (uint16_t bi = 0; bi < grp->batches.size(); ++bi) {
                const auto& b = grp->batches[bi];
                if (bi < trans_end) continue;
                uint8_t a = (bi < int_end) ? 0u : 255u;
                uint16_t v0 = b.first_vertex;
                uint16_t v1 = b.last_vertex;
                if (v1 >= grp->colors1.size()) v1 = static_cast<uint16_t>(grp->colors1.size() - 1);
                for (uint16_t v = v0; v <= v1; ++v) {
                    grp->colors1[v] = (grp->colors1[v] & 0x00FFFFFFu) |
                                       (static_cast<uint32_t>(a) << 24);
                }
            }
        }
    }

    return grp;
}

} // namespace mve
