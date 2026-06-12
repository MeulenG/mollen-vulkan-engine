#include "adt_types.h"
#include "chunk_handler.h"

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
                    uint32_t tex_idx = ReadU32(mcly_body, l * 16);
                    out.layer_texture[l] = static_cast<int>(tex_idx);
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
                size_t len = std::strlen(p);
                if (len == 0) { p++; continue; }
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

    return mcnk_seen > 0;
}

} // namespace mve
