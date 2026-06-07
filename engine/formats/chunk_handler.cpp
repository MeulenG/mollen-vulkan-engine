#include "chunk_handler.h"

#include <cstring>

namespace mve {

namespace {

// WoW chunk IDs are stored reversed on disk
ChunkId ToChunkId(const uint8_t* p) {
    if (p[0]==0x52 && p[1]==0x45 && p[2]==0x56 && p[3]==0x4D) return ChunkId::MVER; // REVM
    if (p[0]==0x44 && p[1]==0x48 && p[2]==0x50 && p[3]==0x4D) return ChunkId::MPHD; // DHPM
    if (p[0]==0x4E && p[1]==0x49 && p[2]==0x41 && p[3]==0x4D) return ChunkId::MAIN; // NIAM
    if (p[0]==0x4F && p[1]==0x4D && p[2]==0x57 && p[3]==0x4D) return ChunkId::MWMO; // OMWM
    if (p[0]==0x46 && p[1]==0x44 && p[2]==0x4F && p[3]==0x4D) return ChunkId::MODF; // FDOM
    // ADT chunks
    if (p[0]==0x58 && p[1]==0x45 && p[2]==0x54 && p[3]==0x4D) return ChunkId::MTEX; // XETM
    if (p[0]==0x4B && p[1]==0x4E && p[2]==0x43 && p[3]==0x4D) return ChunkId::MCNK; // KNCM
    if (p[0]==0x54 && p[1]==0x56 && p[2]==0x43 && p[3]==0x4D) return ChunkId::MCVT; // TVCM
    if (p[0]==0x59 && p[1]==0x4C && p[2]==0x43 && p[3]==0x4D) return ChunkId::MCLY; // YLCM
    return ChunkId::Unknown;
}

} // namespace

bool WalkChunks(const uint8_t* buf, size_t total, const std::unordered_map<ChunkId, ChunkHandler>& handlers) {
    size_t pos = 0;
    while (pos + 8 <= total) {
        const uint8_t* chunk_start = buf + pos;
        uint32_t payload_size;
        std::memcpy(&payload_size, chunk_start + 4, 4);
        pos += 8;
        if (pos + payload_size > total) return false;
        auto it = handlers.find(ToChunkId(chunk_start));
        if (it != handlers.end()) {
            if (!it->second(buf + pos, payload_size)) return false;
        }
        pos += payload_size;
    }
    return true;
}

} // namespace mve