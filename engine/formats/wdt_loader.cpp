#include "wdt_loader.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace mve {

namespace {

// WoW chunk IDs are stored reversed on disk (e.g. "MVER" appears as bytes
// 'R','E','V','M'). Read as little-endian uint32 that gives 'R' in LSB,
// 'M' in MSB. To match, the FourCC builder puts s[3] in the LSB.
constexpr uint32_t FourCC(const char* s) {
    return (uint32_t)(uint8_t)s[3]
         | ((uint32_t)(uint8_t)s[2] << 8)
         | ((uint32_t)(uint8_t)s[1] << 16)
         | ((uint32_t)(uint8_t)s[0] << 24);
}

} // namespace

bool WdtLoader::LoadFile(const std::string& path,
                          std::vector<WdtTile>& out_existing_tiles) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "WdtLoader: cannot open %s\n", path.c_str());
        return false;
    }
    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0) return false;

    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (!in.read(reinterpret_cast<char*>(buf.data()), size)) return false;

    out_existing_tiles.clear();

    // Walk top-level chunks. Each chunk is { uint32 id, uint32 size,
    // uint8 payload[size] }.
    size_t pos = 0;
    while (pos + 8 <= buf.size()) {
        uint32_t id = 0, payload_size = 0;
        std::memcpy(&id, &buf[pos], 4);
        std::memcpy(&payload_size, &buf[pos + 4], 4);
        pos += 8;
        if (pos + payload_size > buf.size()) break;

        if (id == FourCC("MAIN")) {
            // 64*64 entries, each 8 bytes. The low bit of the first uint32
            // per entry indicates "tile exists".
            constexpr int kGrid = 64;
            if (payload_size < static_cast<uint32_t>(kGrid * kGrid * 8)) break;
            const uint8_t* p = &buf[pos];
            for (int y = 0; y < kGrid; y++) {
                for (int x = 0; x < kGrid; x++) {
                    uint32_t flags = 0;
                    std::memcpy(&flags, p + (y * kGrid + x) * 8, 4);
                    if (flags & 0x1) {
                        out_existing_tiles.push_back({ x, y });
                    }
                }
            }
        }
        // Other top-level chunks (MVER, MPHD, MWMO, MODF) are ignored;
        // we only need MAIN for R1.

        pos += payload_size;
    }

    return true;
}

} // namespace mve
