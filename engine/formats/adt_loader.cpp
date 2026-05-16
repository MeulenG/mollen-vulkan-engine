#include "adt_types.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace mve {

namespace {

// See wdt_loader.cpp for the rationale on FourCC matching - WoW chunk IDs
// are stored reversed on disk and a naive FourCC literal matches the
// little-endian read.
constexpr uint32_t FourCC(const char* s) {
    return (uint32_t)(uint8_t)s[0]
         | ((uint32_t)(uint8_t)s[1] << 8)
         | ((uint32_t)(uint8_t)s[2] << 16)
         | ((uint32_t)(uint8_t)s[3] << 24);
}

// Layout of the MCNK header for ADT v18. Only the fields R1 needs are
// extracted; everything else is skipped via offsets. References:
// https://wowdev.wiki/ADT/v18#MCNK_header
struct McnkHeader {
    uint32_t flags;
    uint32_t index_x;
    uint32_t index_y;
    uint32_t n_layers;
    uint32_t n_doodad_refs;
    uint32_t ofs_mcvt;          // offset to MCVT sub-chunk, relative to MCNK chunk start
    uint32_t ofs_mcnr;
    uint32_t ofs_mcly;
    uint32_t ofs_mcrf;
    uint32_t ofs_mcal;
    uint32_t size_alpha;
    uint32_t ofs_mcsh;
    uint32_t size_shadow;
    uint32_t area_id;
    uint32_t n_map_obj_refs;
    uint32_t holes_low_res;
    // ... more fields we don't need
    // The (position_y, position_x, position_z) triple lives at offset 104.
    // WoW stores chunks with: position_y = north-south coord (X axis WoW),
    //                         position_x = east-west coord (Y axis WoW),
    //                         position_z = base height.
};

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

// Parse a single MCNK. `mcnk_data` points at the byte immediately after
// the MCNK chunk's 8-byte top-level header (i.e. the MCNK *payload*).
// `mcnk_payload_size` is the payload length.
bool ParseMcnk(const uint8_t* mcnk_data, size_t mcnk_payload_size,
                AdtChunk& out) {
    if (mcnk_payload_size < 128) return false;  // MCNK header is 128 bytes

    // Chunk world position. WoW stores them as (position_y, position_x, position_z)
    // at offset 104 within the MCNK header. We translate to our struct's
    // (wow_x, wow_y, wow_z_base) where wow_x is the south-pointing axis
    // and wow_y is the east-pointing axis.
    out.wow_y      = ReadF32(mcnk_data, 104);  // east
    out.wow_x      = ReadF32(mcnk_data, 108);  // south
    out.wow_z_base = ReadF32(mcnk_data, 112);  // height base

    uint32_t n_layers   = ReadU32(mcnk_data, 12);
    uint32_t ofs_mcvt   = ReadU32(mcnk_data, 20);
    uint32_t ofs_mcly   = ReadU32(mcnk_data, 28);
    if (ofs_mcvt == 0) return false;

    // MCVT sub-chunk: { id, size, 145 floats }. ofs_mcvt is from MCNK
    // chunk start (the 'MCNK' header), so the sub-chunk starts at
    // mcnk_data + (ofs_mcvt - 8) within the payload.
    //
    // Actually MCNK sub-chunk offsets in ADT v18 are documented as "from
    // start of MCNK chunk including its header" - so to index into payload
    // we subtract 8. Subtracting 8 is the standard convention.
    size_t mcvt_off = (size_t)ofs_mcvt - 8;
    if (mcvt_off + 8 + kAdtVertsPerChunk * 4 > mcnk_payload_size) return false;

    uint32_t mcvt_id   = ReadU32(mcnk_data, mcvt_off);
    if (mcvt_id != FourCC("MCVT")) return false;

    const uint8_t* mcvt_body = mcnk_data + mcvt_off + 8;
    // MCVT layout: 145 floats, interleaved as 9, 8, 9, 8, 9, 8, 9, 8, 9.
    // We split into outer (81 values) and inner (64 values) per the
    // AdtChunkHeights struct.
    int outer_i = 0, inner_i = 0;
    int row = 0;
    // We have 17 mini-rows: 9 outer + 8 inner alternating.
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
    (void)row;

    // Capture layer texture indices for future texturing work.
    out.layer_count = static_cast<int>(n_layers > 4 ? 4 : n_layers);
    if (ofs_mcly > 0 && n_layers > 0) {
        size_t mcly_off = (size_t)ofs_mcly - 8;
        if (mcly_off + 8 <= mcnk_payload_size) {
            uint32_t mcly_id   = ReadU32(mcnk_data, mcly_off);
            uint32_t mcly_size = ReadU32(mcnk_data, mcly_off + 4);
            if (mcly_id == FourCC("MCLY") &&
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

    // Walk top-level chunks.
    size_t pos = 0;
    while (pos + 8 <= buf.size()) {
        uint32_t id           = ReadU32(buf.data(), pos);
        uint32_t payload_size = ReadU32(buf.data(), pos + 4);
        pos += 8;
        if (pos + payload_size > buf.size()) break;

        if (id == FourCC("MTEX")) {
            // Null-separated strings. Last string is also null-terminated.
            const char* p   = reinterpret_cast<const char*>(&buf[pos]);
            const char* end = p + payload_size;
            while (p < end) {
                size_t len = std::strlen(p);
                if (len == 0) { p++; continue; }
                out.textures.emplace_back(p, len);
                p += len + 1;
            }
        } else if (id == FourCC("MCNK")) {
            // Up to 256 MCNK chunks. Order in the file is row-major (the
            // index_x and index_y inside the header give the canonical
            // location to store the parsed result).
            if (mcnk_seen < kAdtChunksPerTile) {
                AdtChunk parsed{};
                if (ParseMcnk(&buf[pos], payload_size, parsed)) {
                    // index_x / index_y come from the MCNK header (offsets 4/8).
                    uint32_t ix = ReadU32(&buf[pos], 4);
                    uint32_t iy = ReadU32(&buf[pos], 8);
                    if (ix < 16 && iy < 16) {
                        out.chunks[iy * 16 + ix] = parsed;
                    }
                }
            }
            mcnk_seen++;
        }
        // MVER / MHDR / MCIN / MMDX / MMID / MWMO / MWID / MDDF / MODF /
        // MH2O / MFBO / MTXF: ignored for R1.

        pos += payload_size;
    }

    return mcnk_seen > 0;
}

} // namespace mve
