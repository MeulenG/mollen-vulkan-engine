#include "adt_types.h"
#include "chunk_handler.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <glm/vec3.hpp>

namespace mve {

namespace {

// Full MCNK header layout per https://wowdev.wiki/ADT/v18#MCNK_chunk (SMChunk).
struct SMChunk
{
/*0x000*/  struct
           {
               uint32_t has_mcsh           : 1;
               uint32_t impass             : 1;
               uint32_t lq_river           : 1;
               uint32_t lq_ocean           : 1;
               uint32_t lq_magma           : 1;
               uint32_t lq_slime           : 1;
               uint32_t has_mccv           : 1;
               uint32_t unknown_0x80       : 1;
               uint32_t                    : 7;
               uint32_t do_not_fix_alpha_map : 1;
               uint32_t high_res_holes     : 1;  // MoP ~5.3+: use holes_high_res field instead of holes_low_res
               uint32_t                    : 15;
           } flags;
/*0x004*/  uint32_t IndexX;
/*0x008*/  uint32_t IndexY;
/*0x00C*/  uint32_t nLayers;             // max 4 (pre-Midnight)
/*0x010*/  uint32_t nDoodadRefs;
/*0x014*/  uint32_t ofsHeight;           // WotLK: offset to MCVT. MoP+: lower 32 bits of holes_high_res.
/*0x018*/  uint32_t ofsNormal;           // WotLK: offset to MCNR. MoP+: upper 32 bits of holes_high_res.
/*0x01C*/  uint32_t ofsLayer;            // offset to MCLY
/*0x020*/  uint32_t ofsRefs;             // offset to MCRF
/*0x024*/  uint32_t ofsAlpha;            // offset to MCAL
/*0x028*/  uint32_t sizeAlpha;
/*0x02C*/  uint32_t ofsShadow;           // offset to MCSH (only if flags.has_mcsh)
/*0x030*/  uint32_t sizeShadow;
/*0x034*/  uint32_t areaid;
/*0x038*/  uint32_t nMapObjRefs;
/*0x03C*/  uint16_t holes_low_res;       // 16-bit hole bitmap (use when !flags.high_res_holes)
/*0x03E*/  uint16_t unknown_but_used;
/*0x040*/  uint8_t  predominant_texture[16]; // uint2_t[8][8] packed: 4 values per byte, 2 bits each
                                             // read: (predominant_texture[row*2 + col/4] >> ((col%4)*2)) & 0x3
/*0x050*/  uint8_t  no_effect_doodad[8];    // uint1_t[8][8] packed: 8 values per byte, 1 bit each
                                             // read: (no_effect_doodad[row] >> col) & 0x1
/*0x058*/  uint32_t ofsSndEmitters;
/*0x05C*/  uint32_t nSndEmitters;
/*0x060*/  uint32_t ofsLiquid;
/*0x064*/  uint32_t sizeLiquid;          // 8 when unused
/*0x068*/  glm::vec3 position;           // WoW coords: x=Y(east), y=X(south), z=height
/*0x074*/  uint32_t ofsMCCV;            // WotLK+: offset to MCCV (only if flags.has_mccv)
/*0x078*/  uint32_t ofsMCLV;            // Cata+: offset to MCLV
/*0x07C*/  uint32_t unused;
/*0x080*/
};

static_assert(sizeof(SMChunk) == 128, "SMChunk size mismatch — check field types");

// Reads a little-endian uint32 from buf at offset off.
inline uint32_t ReadU32(const uint8_t* buf, size_t off) {
    uint32_t v = 0;
    std::memcpy(&v, buf + off, 4);
    return v;
}
inline float ReadF32(const uint8_t* buf, size_t off) {
    float v = 0;
    std::memcpy(&v, buf + off, 4);
    return v;
}

// Validate that the sub-chunk at `off` inside the MCNK payload carries the
// expected tag. Sub-chunks are offset-addressed from the MCNK header (not
// sequentially walked), so we check the 4 id bytes in place.
inline bool SubChunkIs(const uint8_t* mcnk_data, size_t off, ChunkId want) {
    return IdentifyChunk(mcnk_data + off) == want;
}

// MCLY layer flags we care about (full list at wowdev.wiki/ADT/v18#MCLY).
// 0x100 (do not fix alpha map) and 0x200 (alpha map is compressed RLE)
// are the ones that change MCAL decoding.
constexpr uint32_t kMclyFlagDoNotFixAlpha = 0x100;
constexpr uint32_t kMclyFlagAlphaCompressed = 0x200;

// Decodes one chunk's 4096-byte alpha map out of MCAL bytes starting at
// `src`. `flags` are the MCLY layer flags. `mhdr_flag_big_alpha` is true
// when MHDR.flags bit 0x4 is set, indicating that uncompressed alpha maps
// are 4096 bytes per layer (one byte per texel). When false (legacy WotLK
// default), uncompressed alphas are 2048 bytes of 4-bit nibbles that we
// expand to 4096 bytes.
//
// `available` is the remaining size of the MCAL body so we don't read past
// the buffer in malformed files.
//
// Returns the number of bytes consumed from src.
size_t DecodeAlphaMap(const uint8_t* src, size_t available,
                      uint32_t flags, bool mhdr_flag_big_alpha,
                      uint8_t out_alpha[64 * 64]) {
    constexpr size_t kTexels = 64 * 64;  // 4096
    if (flags & kMclyFlagAlphaCompressed) {
        // RLE: each command byte's low 7 bits are the run length. If the
        // high bit is set ("fill mode"), the next byte is the value to
        // repeat. Otherwise ("copy mode"), the next `run` bytes are the
        // literal values.
        size_t written = 0;
        size_t p = 0;
        while (written < kTexels && p < available) {
            uint8_t cmd = src[p++];
            uint8_t run = cmd & 0x7F;
            if (run == 0) break;        // malformed; bail
            if (cmd & 0x80) {
                if (p >= available) break;
                uint8_t v = src[p++];
                size_t n = std::min<size_t>(run, kTexels - written);
                std::memset(out_alpha + written, v, n);
                written += n;
            } else {
                size_t n = std::min<size_t>(run, kTexels - written);
                if (p + n > available) break;
                std::memcpy(out_alpha + written, src + p, n);
                p += n;
                written += n;
            }
        }
        // If decompression stopped short, leave the tail at zero (default).
        return p;
    }

    if (mhdr_flag_big_alpha) {
        // 4096 raw bytes - one per texel.
        size_t n = std::min<size_t>(kTexels, available);
        std::memcpy(out_alpha, src, n);
        return n;
    }

    // 2048 bytes, 4 bits per texel. Each byte holds two adjacent texels:
    // low nibble first, high nibble second. Expand to 4096 bytes by
    // mapping nibble (0..15) to byte (0..255) via 17 * nibble.
    size_t want = std::min<size_t>(2048u, available);
    for (size_t i = 0; i < want; i++) {
        uint8_t b = src[i];
        uint8_t lo = b & 0x0F;
        uint8_t hi = (b >> 4) & 0x0F;
        out_alpha[i * 2 + 0] = static_cast<uint8_t>(lo * 17);
        out_alpha[i * 2 + 1] = static_cast<uint8_t>(hi * 17);
    }
    return want;
}

// "Fix alpha" pass for the historical 63x63-with-implicit-last-row format.
// When MCLY.flags & 0x100 is *clear* (the default), WoW expects the 64th
// row and column to be a duplicate of row 62 / column 62.
//
// Many WotLK ADTs use bit 0x100 (do-not-fix) so this is essentially a
// no-op on modern data, but the legacy path costs a handful of byte
// copies and keeps R2 robust against early-content ADTs.
void FixAlphaEdges(uint8_t out_alpha[64 * 64]) {
    // Duplicate row 62 into row 63.
    std::memcpy(out_alpha + 63 * 64, out_alpha + 62 * 64, 64);
    // Duplicate column 62 into column 63 (per row).
    for (int r = 0; r < 64; r++) {
        out_alpha[r * 64 + 63] = out_alpha[r * 64 + 62];
    }
}

// Decode MCNR: 145 signed-byte triplets packed as (nx, nz, ny) per
// wowdev.wiki/ADT/v18#MCNR. The 145 triplets follow the same 9, 8, 9, 8, ...
// row layout as MCVT. Each signed byte in [-127, 127] divided by 127 gives
// the [-1, 1] component.
//
// The on-disk axes are WoW (nx=south, ny=east, nz=up). Engine render is
// Z-up: renderX=canonical.X (north) = -south, renderY=canonical.Y
// (west) = -east, renderZ=canonical.Z (up).
void DecodeMcnr(const uint8_t* mcnr_body, AdtChunkNormals& out) {
    auto read_one = [&](size_t base, float* dst) {
        int8_t nx = static_cast<int8_t>(mcnr_body[base + 0]);
        int8_t nz = static_cast<int8_t>(mcnr_body[base + 1]);
        int8_t ny = static_cast<int8_t>(mcnr_body[base + 2]);
        dst[0] = -static_cast<float>(nx) / 127.0f;   // renderX (north)
        dst[1] = -static_cast<float>(ny) / 127.0f;   // renderY (west)
        dst[2] =  static_cast<float>(nz) / 127.0f;   // renderZ (up)
    };

    int outer_i = 0, inner_i = 0;
    int fi = 0;
    for (int mini = 0; mini < 17; mini++) {
        bool is_outer = (mini % 2 == 0);
        int verts = is_outer ? 9 : 8;
        for (int i = 0; i < verts; i++, fi++) {
            if (is_outer) {
                read_one(fi * 3, &out.n_outer[outer_i * 3]);
                outer_i++;
            } else {
                read_one(fi * 3, &out.n_inner[inner_i * 3]);
                inner_i++;
            }
        }
    }
    out.parsed = true;
}

// Decode MCCV: 145 BGRA8 quadruplets per wowdev.wiki/ADT/v18#MCCV. We
// only consume RGB - the alpha byte historically indicated "fixed
// pipeline lighting mix" and is ignored by modern engines. WoW
// authoring tools paint these as a per-vertex color overlay; the
// terrain shader multiplies them in to introduce the subtle hue
// shifts that the flat splat textures alone can't capture.
//
// Same 9, 8, 9, 8, ... mini-row layout as MCVT / MCNR. RGB bytes
// 0..255 normalize to 0..1; 0x7F (= 0.498) is the "neutral, no tint"
// midpoint that WoW uses when only some vertices in a chunk are
// painted.
void DecodeMccv(const uint8_t* mccv_body, AdtChunkVertexColors& out) {
    auto read_one = [&](size_t base, float* dst) {
        // BGRA on disk; we want RGB in linear order. Note alpha (byte 3)
        // is ignored - kept as a comment for future investigation if
        // someone wants to use it as a height-shading mask.
        uint8_t b = mccv_body[base + 0];
        uint8_t g = mccv_body[base + 1];
        uint8_t r = mccv_body[base + 2];
        dst[0] = static_cast<float>(r) / 255.0f;
        dst[1] = static_cast<float>(g) / 255.0f;
        dst[2] = static_cast<float>(b) / 255.0f;
    };

    int outer_i = 0, inner_i = 0;
    int fi = 0;
    for (int mini = 0; mini < 17; mini++) {
        bool is_outer = (mini % 2 == 0);
        int verts = is_outer ? 9 : 8;
        for (int i = 0; i < verts; i++, fi++) {
            if (is_outer) {
                read_one(fi * 4, &out.c_outer[outer_i * 3]);
                outer_i++;
            } else {
                read_one(fi * 4, &out.c_inner[inner_i * 3]);
                inner_i++;
            }
        }
    }
    out.parsed = true;
}

// Parse a single MCNK. `mcnk_data` points at the byte immediately after
// the MCNK chunk's 8-byte top-level header (i.e. the MCNK *payload*).
// `mcnk_payload_size` is the payload length.
//
// `mhdr_flag_big_alpha` is the tile-level MHDR.flags bit 0x4 - it changes
// how uncompressed MCAL alpha maps are sized.
bool ParseMcnk(const uint8_t* mcnk_data, size_t mcnk_payload_size,
               bool mhdr_flag_big_alpha, AdtChunk& out) {
    if (mcnk_payload_size < sizeof(SMChunk)) {
        return false;
    }

    SMChunk hdr;
    std::memcpy(&hdr, mcnk_data, sizeof(hdr));

    // position: x=Y(east), y=X(south), z=height base.
    out.wow_y      = hdr.position.x;
    out.wow_x      = hdr.position.y;
    out.wow_z_base = hdr.position.z;
    out.holes_low_res = hdr.holes_low_res;

    if (hdr.ofsHeight == 0) {
        return false;
    }

    // Sub-chunk offsets are from the start of the MCNK chunk including its
    // 8-byte top-level id+size header. Subtract 8 to get offset into the
    // payload we were handed.
    size_t mcvt_off = (size_t)hdr.ofsHeight - 8;
    if (mcvt_off + 8 + kAdtVertsPerChunk * 4 > mcnk_payload_size) return false;
    if (!SubChunkIs(mcnk_data, mcvt_off, ChunkId::MCVT)) return false;

    const uint8_t* mcvt_body = mcnk_data + mcvt_off + 8;
    // MCVT layout: 145 floats, interleaved as 9, 8, 9, 8, 9, 8, 9, 8, 9.
    // We split into outer (81 values) and inner (64 values) per the
    // AdtChunkHeights struct.
    int outer_i = 0, inner_i = 0;
    // 17 mini-rows: 9 outer + 8 inner alternating.
    for (int mini = 0, fi = 0; mini < 17; mini++) {
        bool is_outer = (mini % 2 == 0);
        int verts = is_outer ? 9 : 8;
        for (int i = 0; i < verts; i++, fi++) {
            float rel = 0.0f;
            std::memcpy(&rel, mcvt_body + fi * 4, 4);
            float abs_h = out.wow_z_base + rel;
            if (is_outer) out.heights.y_outer[outer_i++] = abs_h;
            else          out.heights.y_inner[inner_i++] = abs_h;
        }
    }

    // MCNR sub-chunk: 145 packed normals. Optional; absence falls back to
    // mesh-time normal aggregation.
    if (hdr.ofsNormal > 0) {
        size_t mcnr_off = (size_t)hdr.ofsNormal - 8;
        if (mcnr_off + 8 + 145 * 3 <= mcnk_payload_size &&
            SubChunkIs(mcnk_data, mcnr_off, ChunkId::MCNR)) {
            DecodeMcnr(mcnk_data + mcnr_off + 8, out.normals);
        }
    }

    // MCCV sub-chunk: 145 BGRA8 per-vertex tints. Optional - older ADTs
    // or unpainted chunks don't have it; out.vertex_colors.parsed stays
    // false and the mesh builder uses neutral white.
    if (hdr.ofsMCCV > 0) {
        size_t mccv_off = (size_t)hdr.ofsMCCV - 8;
        if (mccv_off + 8 + 145 * 4 <= mcnk_payload_size &&
            SubChunkIs(mcnk_data, mccv_off, ChunkId::MCCV)) {
            DecodeMccv(mcnk_data + mccv_off + 8, out.vertex_colors);
        }
    }

    // MCLY sub-chunk: 16 bytes per layer:
    //   texture_index (uint32 @ +0), flags (uint32 @ +4),
    //   ofs_mcal      (uint32 @ +8), effect_id (uint32 @ +12)
    // effect_id is the GroundEffectTexture.dbc row id - the key the
    // detail-grass system uses to look up which grass M2s to scatter
    // on this layer. 0 means "no ground cover for this layer".
    out.layer_count = static_cast<int>(hdr.nLayers > 4 ? 4 : hdr.nLayers);

    // Track each layer's MCAL offset so the MCAL decode pass below can
    // find each layer's alpha block. Stored locally because AdtLayer
    // hides the layout once we've decoded.
    uint32_t per_layer_ofs_mcal[4] = {0, 0, 0, 0};

    if (hdr.ofsLayer > 0 && hdr.nLayers > 0) {
        size_t mcly_off = (size_t)hdr.ofsLayer - 8;
        if (mcly_off + 8 <= mcnk_payload_size) {
            uint32_t mcly_size = ReadU32(mcnk_data, mcly_off + 4);
            if (SubChunkIs(mcnk_data, mcly_off, ChunkId::MCLY) &&
                mcly_off + 8 + mcly_size <= mcnk_payload_size) {
                const uint8_t* mcly_body = mcnk_data + mcly_off + 8;
                for (int l = 0; l < out.layer_count; l++) {
                    uint32_t tex_idx   = ReadU32(mcly_body, l * 16 + 0);
                    uint32_t flags     = ReadU32(mcly_body, l * 16 + 4);
                    uint32_t ofs_a     = ReadU32(mcly_body, l * 16 + 8);
                    uint32_t effect_id = ReadU32(mcly_body, l * 16 + 12);
                    out.layers[l].texture_index = static_cast<int>(tex_idx);
                    out.layers[l].flags = flags;
                    out.layers[l].effect_id = effect_id;
                    per_layer_ofs_mcal[l] = ofs_a;
                }
            }
        }
    }

    // MCAL sub-chunk: contiguous blob holding all upper-layer alpha maps.
    // Each layer's offset within MCAL comes from MCLY. Layer 0 has no
    // alpha map (it's always full coverage); we skip it.
    if (hdr.ofsAlpha > 0 && out.layer_count > 1) {
        size_t mcal_off = (size_t)hdr.ofsAlpha - 8;
        if (mcal_off + 8 <= mcnk_payload_size &&
            SubChunkIs(mcnk_data, mcal_off, ChunkId::MCAL)) {
            const uint8_t* mcal_body = mcnk_data + mcal_off + 8;
            size_t mcal_body_max = mcnk_payload_size - (mcal_off + 8);

            for (int l = 1; l < out.layer_count; l++) {
                uint32_t lof = per_layer_ofs_mcal[l];
                if (lof >= mcal_body_max) continue;
                DecodeAlphaMap(mcal_body + lof,
                               mcal_body_max - lof,
                               out.layers[l].flags,
                               mhdr_flag_big_alpha,
                               out.layers[l].alpha);
                if ((out.layers[l].flags & kMclyFlagDoNotFixAlpha) == 0) {
                    FixAlphaEdges(out.layers[l].alpha);
                }
            }
        }
    }

    return true;
}

} // namespace

bool AdtLoader::LoadFile(const std::string& path, AdtTile& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "AdtLoader: cannot open %s\n", path.c_str());
        return false;
    }
    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0) return false;

    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (!in.read(reinterpret_cast<char*>(buf.data()), size)) return false;

    int mcnk_seen = 0;

    // MHDR.flags bit 0x4 selects between the 4096-byte and 2048-byte
    // uncompressed alpha-map size. We default to false (legacy 2048-byte
    // nibble-packed format) and flip it if we see the bit set in MHDR.
    // MHDR precedes all MCNKs in the file, and WalkChunks visits chunks
    // in file order, so the flag is settled before the first MCNK fires.
    bool mhdr_flag_big_alpha = false;

    // MMDX/MMID/MDDF can appear in any order. Capture the raw bytes
    // here and resolve into out.doodad_paths after the main walk.
    std::vector<uint8_t> mmdx_blob;
    std::vector<uint32_t> mmid_offsets;

    // MWMO/MWID/MODF: same shape as the doodad pair but for WMO
    // buildings. Captured here, resolved after the main walk.
    std::vector<uint8_t> mwmo_blob;
    std::vector<uint32_t> mwid_offsets;

    WalkChunks(buf.data(), buf.size(), {
        { ChunkId::MHDR, [&](const uint8_t* data, uint32_t payload_size) {
            // MHDR starts with a flags uint32. Bit 0x4 = "uses big alpha"
            // (4096 bytes per uncompressed layer rather than 2048 nibbles).
            if (payload_size >= 4) {
                uint32_t mhdr_flags = ReadU32(data, 0);
                mhdr_flag_big_alpha = (mhdr_flags & 0x4) != 0;
            }
            return true;
        }},
        { ChunkId::MTEX, [&](const uint8_t* data, uint32_t payload_size) {
            // Null-separated strings. Last string is also null-terminated.
            // memchr instead of strlen so a malformed blob with a missing
            // final terminator can't read past the payload.
            const char* p   = reinterpret_cast<const char*>(data);
            const char* end = p + payload_size;
            while (p < end) {
                size_t remaining = static_cast<size_t>(end - p);
                const void* nul = std::memchr(p, '\0', remaining);
                if (!nul) break;
                size_t len = static_cast<const char*>(nul) - p;
                if (len == 0) { p++; continue; }
                out.textures.emplace_back(p, len);
                p += len + 1;
            }
            return true;
        }},
        { ChunkId::MMDX, [&](const uint8_t* data, uint32_t payload_size) {
            // Null-separated M2 path strings. Byte-addressable - MMID
            // entries are byte offsets, not string indices.
            mmdx_blob.assign(data, data + payload_size);
            return true;
        }},
        { ChunkId::MMID, [&](const uint8_t* data, uint32_t payload_size) {
            uint32_t count = payload_size / 4;
            mmid_offsets.resize(count);
            for (uint32_t i = 0; i < count; i++) {
                mmid_offsets[i] = ReadU32(data, i * 4);
            }
            return true;
        }},
        { ChunkId::MDDF, [&](const uint8_t* data, uint32_t payload_size) {
            // 36-byte entries per wowdev.wiki/ADT/v18#MDDF.
            uint32_t count = payload_size / 36;
            out.doodads.reserve(count);
            for (uint32_t i = 0; i < count; i++) {
                size_t off = i * 36;
                AdtDoodadPlacement d{};
                d.name_id   = ReadU32(data, off + 0);
                d.unique_id = ReadU32(data, off + 4);
                d.pos_x     = ReadF32(data, off + 8);
                d.pos_y     = ReadF32(data, off + 12);
                d.pos_z     = ReadF32(data, off + 16);
                d.rot_x_deg = ReadF32(data, off + 20);
                d.rot_y_deg = ReadF32(data, off + 24);
                d.rot_z_deg = ReadF32(data, off + 28);
                uint32_t packed = ReadU32(data, off + 32);
                uint16_t s_raw  = static_cast<uint16_t>(packed & 0xFFFF);
                d.flags = static_cast<uint16_t>((packed >> 16) & 0xFFFF);
                // Scale stored as fixed-point /1024. Clamp to a sane
                // window so a corrupt 0xFFFF -> 64x doesn't explode the
                // scene; real props use 0.5x .. 4x at most.
                float s = s_raw / 1024.0f;
                if (s < 0.01f) s = 0.01f;
                if (s > 100.0f) s = 100.0f;
                d.scale = s;
                out.doodads.push_back(d);
            }
            return true;
        }},
        { ChunkId::MWMO, [&](const uint8_t* data, uint32_t payload_size) {
            // Null-separated WMO path strings. Same byte-addressable
            // convention as MMDX - MWID stores byte offsets.
            mwmo_blob.assign(data, data + payload_size);
            return true;
        }},
        { ChunkId::MWID, [&](const uint8_t* data, uint32_t payload_size) {
            uint32_t count = payload_size / 4;
            mwid_offsets.resize(count);
            for (uint32_t i = 0; i < count; i++) {
                mwid_offsets[i] = ReadU32(data, i * 4);
            }
            return true;
        }},
        { ChunkId::MODF, [&](const uint8_t* data, uint32_t payload_size) {
            // 64-byte entries per wowdev.wiki/ADT/v18#MODF. Same
            // pos/rot/scale layout as MDDF but with an extra AABB +
            // doodad_set + name_set + flags. WoW 3.3.5a stores scale
            // as 1024 (= 1.0) effectively constant.
            uint32_t count = payload_size / 64;
            out.wmos.reserve(count);
            for (uint32_t i = 0; i < count; i++) {
                size_t off = i * 64;
                AdtWmoPlacement w{};
                w.name_id      = ReadU32(data, off + 0);
                w.unique_id    = ReadU32(data, off + 4);
                w.pos_x        = ReadF32(data, off + 8);
                w.pos_y        = ReadF32(data, off + 12);
                w.pos_z        = ReadF32(data, off + 16);
                w.rot_x_deg    = ReadF32(data, off + 20);
                w.rot_y_deg    = ReadF32(data, off + 24);
                w.rot_z_deg    = ReadF32(data, off + 28);
                w.bbox_lo[0]   = ReadF32(data, off + 32);
                w.bbox_lo[1]   = ReadF32(data, off + 36);
                w.bbox_lo[2]   = ReadF32(data, off + 40);
                w.bbox_hi[0]   = ReadF32(data, off + 44);
                w.bbox_hi[1]   = ReadF32(data, off + 48);
                w.bbox_hi[2]   = ReadF32(data, off + 52);
                w.flags        = static_cast<uint16_t>(
                    ReadU32(data, off + 56) & 0xFFFF);
                w.doodad_set   = static_cast<uint16_t>(
                    (ReadU32(data, off + 56) >> 16) & 0xFFFF);
                w.name_set     = static_cast<uint16_t>(
                    ReadU32(data, off + 60) & 0xFFFF);
                w.scale        = static_cast<uint16_t>(
                    (ReadU32(data, off + 60) >> 16) & 0xFFFF);
                out.wmos.push_back(w);
            }
            return true;
        }},
        { ChunkId::MH2O, [&](const uint8_t* base, uint32_t payload_size) {
            // Top-level liquid chunk (TBC+ format, used by all WotLK ADTs).
            // Layout (per wowdev.wiki/ADT/v18#MH2O and WebWowViewerCpp
            // adtFileHeader.h SMLiquidChunk):
            //
            //   [0 .. 256 * 12)         = 256 SMLiquidChunk headers
            //   [variable .. variable)  = SMLiquidInstance[] (24 B each)
            //   [variable .. variable)  = mh2o_chunk_attributes (16 B)
            //   [variable .. variable)  = exists bitmaps (variable B)
            //   [variable .. variable)  = vertex data (LVF-dependent)
            //
            // All sub-offsets in the headers are RELATIVE to the start of
            // the MH2O payload (not file-relative).
            size_t base_size = payload_size;
            for (uint32_t ci = 0; ci < 256u; ++ci) {
                if (ci * 12u + 12u > base_size) break;
                uint32_t ofs_inst = ReadU32(base, ci * 12u + 0);
                uint32_t n_layers = ReadU32(base, ci * 12u + 4);
                // ofs_attributes at +8 - we skip attributes (fishable/deep
                // bitmasks) for v1; depth comes from vertex data instead.
                for (uint32_t li = 0; li < n_layers; ++li) {
                    size_t io = ofs_inst + li * 24u;
                    if (io + 24u > base_size) break;
                    AdtLiquidInstance inst{};
                    inst.chunk_index = static_cast<uint16_t>(ci);
                    inst.liquid_type = static_cast<uint16_t>(
                        ReadU32(base, io + 0) & 0xFFFFu);
                    uint16_t lvf_or_obj = static_cast<uint16_t>(
                        (ReadU32(base, io + 0) >> 16) & 0xFFFFu);
                    // In WotLK Elwynn, this is always the LVF directly
                    // (values <= 41). LiquidObject.dbc lookups only kick
                    // in at >= 42 which Cata+ used.
                    inst.lvf        = lvf_or_obj <= 41 ? lvf_or_obj : 0;
                    inst.min_height = ReadF32(base, io + 4);
                    inst.max_height = ReadF32(base, io + 8);
                    inst.x_offset   = base[io + 12];
                    inst.y_offset   = base[io + 13];
                    inst.width      = base[io + 14];
                    inst.height     = base[io + 15];
                    uint32_t ofs_exists = ReadU32(base, io + 16);
                    uint32_t ofs_verts  = ReadU32(base, io + 20);

                    // Clamp width/height defensively. The spec allows up
                    // to 8; malformed values would overflow our buffers.
                    if (inst.width  > 8) inst.width  = 8;
                    if (inst.height > 8) inst.height = 8;
                    if (inst.width  == 0 || inst.height == 0) continue;

                    uint32_t n_verts = static_cast<uint32_t>(inst.width + 1) *
                                       static_cast<uint32_t>(inst.height + 1);
                    inst.heights.resize(n_verts, inst.min_height);
                    inst.depths.resize(n_verts, 1.0f);

                    // LVF=0 (Elwynn): float[N] heights, then uint8[N] depths.
                    // LVF=1: float[N] heights, then int16[N*2] UVs. No depth.
                    // LVF=2 (ocean): uint8[N] depths only, height = min.
                    // LVF=3: float[N] heights, then int16[N*2] UVs, then uint8[N] depths.
                    if (ofs_verts != 0 && ofs_verts < base_size) {
                        size_t vo = ofs_verts;
                        if (inst.lvf == 0 || inst.lvf == 1 || inst.lvf == 3) {
                            if (vo + n_verts * 4u <= base_size) {
                                for (uint32_t i = 0; i < n_verts; ++i) {
                                    inst.heights[i] = ReadF32(base, vo + i * 4u);
                                }
                                vo += n_verts * 4u;
                            }
                            if (inst.lvf == 1 || inst.lvf == 3) {
                                vo += n_verts * 4u;  // skip int16 s,t UVs
                            }
                            if (inst.lvf == 0 || inst.lvf == 3) {
                                if (vo + n_verts <= base_size) {
                                    for (uint32_t i = 0; i < n_verts; ++i) {
                                        inst.depths[i] = base[vo + i] / 255.0f;
                                    }
                                }
                            }
                        } else if (inst.lvf == 2) {
                            if (vo + n_verts <= base_size) {
                                for (uint32_t i = 0; i < n_verts; ++i) {
                                    inst.depths[i] = base[vo + i] / 255.0f;
                                }
                            }
                        }
                    }

                    // Exists bitmap: (width*height + 7) / 8 bytes. When
                    // ofs_exists == 0 the wiki convention is "all cells
                    // exist" - we leave inst.exists_bits empty and the
                    // mesh builder treats that as a full bitmap.
                    if (ofs_exists != 0) {
                        size_t n_bits  = static_cast<size_t>(inst.width) *
                                         static_cast<size_t>(inst.height);
                        size_t n_bytes = (n_bits + 7u) / 8u;
                        if (ofs_exists + n_bytes <= base_size) {
                            inst.exists_bits.assign(
                                base + ofs_exists,
                                base + ofs_exists + n_bytes);
                        }
                    }

                    out.liquids.push_back(std::move(inst));
                }
            }
            return true;
        }},
        { ChunkId::MCNK, [&](const uint8_t* data, uint32_t payload_size) {
            // Up to 256 MCNK chunks. Order in the file is row-major (the
            // IndexX and IndexY inside the header give the canonical
            // location to store the parsed result).
            if (mcnk_seen < kAdtChunksPerTile &&
                payload_size >= sizeof(SMChunk)) {
                SMChunk hdr;
                std::memcpy(&hdr, data, sizeof(hdr));
                AdtChunk parsed{};
                if (ParseMcnk(data, payload_size,
                              mhdr_flag_big_alpha, parsed)) {
                    if (hdr.IndexX < 16 && hdr.IndexY < 16) {
                        out.chunks[hdr.IndexY * 16 + hdr.IndexX] = parsed;
                    }
                }
            }
            mcnk_seen++;
            return true;
        }},
        // MVER / MCIN / MFBO / MTXF: ignored.
    });

    // Resolve MMID byte offsets into doodad_paths. Done after the main
    // chunk loop so the parse doesn't depend on MMDX/MMID/MDDF order.
    // strnlen with a bound defends against a missing null terminator
    // in malformed MMDX blobs.
    out.doodad_paths.reserve(mmid_offsets.size());
    for (uint32_t off : mmid_offsets) {
        if (off >= mmdx_blob.size()) {
            out.doodad_paths.emplace_back();   // empty -> spawn loop skips
            continue;
        }
        const char* s = reinterpret_cast<const char*>(mmdx_blob.data() + off);
        size_t max_len = mmdx_blob.size() - off;
        out.doodad_paths.emplace_back(s, strnlen(s, max_len));
    }

    // Same trick for WMO paths.
    out.wmo_paths.reserve(mwid_offsets.size());
    for (uint32_t off : mwid_offsets) {
        if (off >= mwmo_blob.size()) {
            out.wmo_paths.emplace_back();
            continue;
        }
        const char* s = reinterpret_cast<const char*>(mwmo_blob.data() + off);
        size_t max_len = mwmo_blob.size() - off;
        out.wmo_paths.emplace_back(s, strnlen(s, max_len));
    }

    return mcnk_seen > 0;
}

} // namespace mve
