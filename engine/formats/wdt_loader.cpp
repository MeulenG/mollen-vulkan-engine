#include "wdt_loader.h"
#include "chunk_handler.h"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace mve {

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

    WalkChunks(buf.data(), buf.size(), {
        { ChunkId::MAIN, [&](const uint8_t* data, uint32_t payload_size) {
            // 64*64 entries, each 8 bytes. The low bit of the first uint32
            // per entry indicates "tile exists".
            constexpr int kGrid = 64;
            if (payload_size < static_cast<uint32_t>(kGrid * kGrid * 8)) return true;
            for (int y = 0; y < kGrid; y++) {
                for (int x = 0; x < kGrid; x++) {
                    uint32_t flags = 0;
                    std::memcpy(&flags, data + (y * kGrid + x) * 8, 4);
                    if (flags & 0x1) {
                        out_existing_tiles.push_back({ x, y });
                    }
                }
            }
            return true;
        }},
        // MVER, MPHD, MWMO, MODF: ignored for R1.
    });

    return true;
}

} // namespace mve
