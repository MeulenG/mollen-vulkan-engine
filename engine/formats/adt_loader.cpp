#include "adt_types.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace mve {

namespace {

// Build a uint32 tag matching the on-disk byte order of a WoW chunk ID.
//
// WoW stores chunk IDs reversed (e.g. "MVER" appears as bytes 'R','E','V','M'
// on disk). When we read those 4 bytes as a little-endian uint32 we get
// 'R' in the LSB, 'M' in the MSB. To match, we build the same value out of
// the C-string literal by putting s[3] (last char) in the LSB.
//
// Without the reverse, every chunk match returned false and the loader
// reported 0 MCNK chunks parsed - which we just discovered while diagnosing
// R1's "nothing renders" bug.
constexpr uint32_t FourCC(const char* s) {
    return (uint32_t)(uint8_t)s[3]
         | ((uint32_t)(uint8_t)s[2] << 8)
         | ((uint32_t)(uint8_t)s[1] << 16)
         | ((uint32_t)(uint8_t)s[0] << 24);
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
// The on-disk axes are WoW (X south, Y east, Z up). We remap to engine
// space (X east, Y up, Z south) at parse time so the mesh builder can use
// the values directly.
void DecodeMcnr(const uint8_t* mcnr_body, AdtChunkNormals& out) {
    auto read_one = [&](size_t base, float* dst) {
        int8_t nx = static_cast<int8_t>(mcnr_body[base + 0]);
        int8_t nz = static_cast<int8_t>(mcnr_body[base + 1]);
        int8_t ny = static_cast<int8_t>(mcnr_body[base + 2]);
        // WoW: (nx=south, ny=east, nz=up).
        // Engine: (X=east, Y=up, Z=south) -> (ny, nz, nx) / 127.
        dst[0] = static_cast<float>(ny) / 127.0f;
        dst[1] = static_cast<float>(nz) / 127.0f;
        dst[2] = static_cast<float>(nx) / 127.0f;
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

// Parse a single MCNK. `mcnk_data` points at the byte immediately after
// the MCNK chunk's 8-byte top-level header (i.e. the MCNK *payload*).
// `mcnk_payload_size` is the payload length.
//
// `mhdr_flag_big_alpha` is the tile-level MHDR.flags bit 0x4 - it changes
// how uncompressed MCAL alpha maps are sized.
bool ParseMcnk(const uint8_t* mcnk_data, size_t mcnk_payload_size,
                bool mhdr_flag_big_alpha, AdtChunk& out) {
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
    uint32_t ofs_mcnr   = ReadU32(mcnk_data, 24);
    uint32_t ofs_mcly   = ReadU32(mcnk_data, 28);
    uint32_t ofs_mcal   = ReadU32(mcnk_data, 40);
    uint32_t size_alpha = ReadU32(mcnk_data, 44);
    out.holes_low_res   = static_cast<uint16_t>(ReadU32(mcnk_data, 60) & 0xFFFFu);
    if (ofs_mcvt == 0) return false;

    // MCVT sub-chunk: { id, size, 145 floats }. ofs_mcvt is from MCNK
    // chunk start (including the 8-byte top-level header), so within the
    // payload we use (ofs_mcvt - 8).
    size_t mcvt_off = (size_t)ofs_mcvt - 8;
    if (mcvt_off + 8 + kAdtVertsPerChunk * 4 > mcnk_payload_size) return false;

    uint32_t mcvt_id   = ReadU32(mcnk_data, mcvt_off);
    if (mcvt_id != FourCC("MCVT")) return false;

    const uint8_t* mcvt_body = mcnk_data + mcvt_off + 8;
    // MCVT layout: 145 floats, interleaved as 9, 8, 9, 8, 9, 8, 9, 8, 9.
    // We split into outer (81 values) and inner (64 values) per the
    // AdtChunkHeights struct.
    int outer_i = 0, inner_i = 0;
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
    if (ofs_mcnr > 0) {
        size_t mcnr_off = (size_t)ofs_mcnr - 8;
        if (mcnr_off + 8 + 145 * 3 <= mcnk_payload_size) {
            uint32_t mcnr_id = ReadU32(mcnk_data, mcnr_off);
            if (mcnr_id == FourCC("MCNR")) {
                DecodeMcnr(mcnk_data + mcnr_off + 8, out.normals);
            }
        }
    }

    // MCLY sub-chunk: 16 bytes per layer. We capture
    // texture_index (uint32 @ +0), flags (uint32 @ +4), and ofs_mcal
    // (uint32 @ +8). effect_id (+12) is unused for now.
    out.layer_count = static_cast<int>(n_layers > 4 ? 4 : n_layers);

    // Track each layer's MCAL offset so the MCAL decode pass below can
    // find each layer's alpha block. Stored locally because AdtLayer
    // hides the layout once we've decoded.
    uint32_t per_layer_ofs_mcal[4] = {0, 0, 0, 0};

    if (ofs_mcly > 0 && n_layers > 0) {
        size_t mcly_off = (size_t)ofs_mcly - 8;
        if (mcly_off + 8 <= mcnk_payload_size) {
            uint32_t mcly_id   = ReadU32(mcnk_data, mcly_off);
            uint32_t mcly_size = ReadU32(mcnk_data, mcly_off + 4);
            if (mcly_id == FourCC("MCLY") &&
                mcly_off + 8 + mcly_size <= mcnk_payload_size) {
                const uint8_t* mcly_body = mcnk_data + mcly_off + 8;
                for (int l = 0; l < out.layer_count; l++) {
                    uint32_t tex_idx = ReadU32(mcly_body, l * 16 + 0);
                    uint32_t flags   = ReadU32(mcly_body, l * 16 + 4);
                    uint32_t ofs_a   = ReadU32(mcly_body, l * 16 + 8);
                    out.layers[l].texture_index = static_cast<int>(tex_idx);
                    out.layers[l].flags = flags;
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

    // MHDR.flags bit 0x4 selects between the 4096-byte and 2048-byte
    // uncompressed alpha-map size. We default to false (legacy 2048-byte
    // nibble-packed format) and flip it if we see the bit set in MHDR.
    bool mhdr_flag_big_alpha = false;

    // Walk top-level chunks.
    size_t pos = 0;
    while (pos + 8 <= buf.size()) {
        uint32_t id           = ReadU32(buf.data(), pos);
        uint32_t payload_size = ReadU32(buf.data(), pos + 4);
        pos += 8;
        if (pos + payload_size > buf.size()) break;

        if (id == FourCC("MHDR")) {
            // MHDR starts with a flags uint32. Bit 0x4 = "uses big alpha"
            // (4096 bytes per uncompressed layer rather than 2048 nibbles).
            if (payload_size >= 4) {
                uint32_t mhdr_flags = ReadU32(&buf[pos], 0);
                mhdr_flag_big_alpha = (mhdr_flags & 0x4) != 0;
            }
        } else if (id == FourCC("MTEX")) {
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
                if (ParseMcnk(&buf[pos], payload_size,
                              mhdr_flag_big_alpha, parsed)) {
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
        // MVER / MCIN / MMDX / MMID / MWMO / MWID / MDDF / MODF /
        // MH2O / MFBO / MTXF: ignored for R2.

        pos += payload_size;
    }

    return mcnk_seen > 0;
}

} // namespace mve
