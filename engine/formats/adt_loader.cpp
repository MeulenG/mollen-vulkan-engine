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

// MCLY layer flags we care about (full list at wowdev.wiki/ADT/v18#MCLY).
// 0x100 (do not fix alpha map) and 0x200 (alpha map is compressed RLE)
// are the ones that change MCAL decoding.
constexpr uint32_t kMclyFlagDoNotFixAlpha = 0x100;
constexpr uint32_t kMclyFlagAlphaCompressed = 0x200;

// MCNK header flag bits. We use flag_mcnk_has_big_alpha (0x4) at the *tile*
// level (via MHDR.flags & 0x4) elsewhere; per-MCNK we don't currently need
// any of the flags here, but the position is documented for future use.

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
bool ParseMcnk(const uint8_t* mcnk_data, size_t mcnk_payload_size, AdtChunk& out) {
    if (mcnk_payload_size < sizeof(SMChunk)) {
        return false;
    } 

    SMChunk hdr;
    std::memcpy(&hdr, mcnk_data, sizeof(hdr));

    // position: x=Y(east), y=X(south), z=height base.
    out.wow_y      = hdr.position.x;
    out.wow_x      = hdr.position.y;
    out.wow_z_base = hdr.position.z;

    if (hdr.ofsHeight == 0) {
        return false;
    }

    // Sub-chunk offsets are from the start of the MCNK chunk including its
    // 8-byte top-level id+size header. Subtract 8 to get offset into the
    // payload we were handed.
    size_t mcvt_off = (size_t)hdr.ofsHeight - 8;
    if (mcvt_off + 8 + kAdtVertsPerChunk * 4 > mcnk_payload_size) return false;

    // "MCVT" is stored reversed on disk as T,V,C,M (0x54,0x56,0x43,0x4D)
    const uint8_t* mcvt_id_p = mcnk_data + mcvt_off;
    if (!(mcvt_id_p[0]==0x54 && mcvt_id_p[1]==0x56 && mcvt_id_p[2]==0x43 && mcvt_id_p[3]==0x4D)) {
        return false;
    }

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
            if (is_outer) {
                out.heights.y_outer[outer_i++] = abs_h; 
            }
            else {
                out.heights.y_inner[inner_i++] = abs_h;
            }
        }
    }

    // Capture layer texture indices for future texturing work.
    out.layer_count = static_cast<int>(hdr.nLayers > 4 ? 4 : hdr.nLayers);
    if (hdr.ofsLayer > 0 && hdr.nLayers > 0) {
        size_t mcly_off = (size_t)hdr.ofsLayer - 8;
        if (mcly_off + 8 <= mcnk_payload_size) {
            // "MCLY" is stored reversed on disk as Y,L,C,M (0x59,0x4C,0x43,0x4D)
            const uint8_t* mcly_id_p = mcnk_data + mcly_off;
            uint32_t mcly_size = ReadU32(mcnk_data, mcly_off + 4);
            if (mcly_id_p[0]==0x59 && mcly_id_p[1]==0x4C && mcly_id_p[2]==0x43 && mcly_id_p[3]==0x4D &&
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
    if (ofs_mcal > 0 && out.layer_count > 1) {
        size_t mcal_off = (size_t)ofs_mcal - 8;
        if (mcal_off + 8 <= mcnk_payload_size) {
            uint32_t mcal_id   = ReadU32(mcnk_data, mcal_off);
            uint32_t mcal_size = ReadU32(mcnk_data, mcal_off + 4);
            (void)mcal_size;
            (void)size_alpha;
            if (mcal_id == FourCC("MCAL")) {
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

    WalkChunks(buf.data(), buf.size(), {
        { ChunkId::MTEX, [&](const uint8_t* data, uint32_t payload_size) {
            // Null-separated strings. Last string is also null-terminated.
            const char* p   = reinterpret_cast<const char*>(data);
            const char* end = p + payload_size;
            while (p < end) {
                size_t remaining = static_cast<size_t>(end - p);
                const void* nul = std::memchr(p, '\0', remaining);
                if (!nul) {
                    break;
                }
                size_t len = static_cast<const char*>(nul) - p;
                if (len == 0) { 
                    p++; continue; 
                }
                out.textures.emplace_back(p, len);
                p += len + 1;
            }
            return true;
        }},
        { ChunkId::MCNK, [&](const uint8_t* data, uint32_t payload_size) {
            // Up to 256 MCNK chunks. Order in the file is row-major (the
            // index_x and index_y inside the header give the canonical
            // location to store the parsed result).
            if (mcnk_seen < kAdtChunksPerTile) {
                SMChunk hdr;
                std::memcpy(&hdr, data, sizeof(hdr));
                AdtChunk parsed{};
                if (ParseMcnk(data, payload_size, parsed)) {
                    if (hdr.IndexX < 16 && hdr.IndexY < 16) {
                        out.chunks[hdr.IndexY * 16 + hdr.IndexX] = parsed;
                    }
                }
            }
            mcnk_seen++;
            return true;
        }},
        // MVER / MHDR / MCIN / MMDX / MMID / MWMO / MWID / MDDF / MODF /
        // MH2O / MFBO / MTXF: ignored for R1.
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
