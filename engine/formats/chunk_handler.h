#ifndef MVE_CHUNK_HANDLER_H
#define MVE_CHUNK_HANDLER_H

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace mve {

enum class ChunkId {
    Unknown,
    // WDT chunks
    MVER,
    MPHD,
    MAIN,
    MWMO,
    MODF,
    // ADT chunks
    MTEX,
    MCNK,
    MCVT,
    MCLY,
};

using ChunkHandler = std::function<bool(const uint8_t* data, uint32_t size)>;

// Walk a flat WoW chunk stream. Each registered handler is called for its
// matching ChunkId. Unknown chunk IDs are silently skipped.
// Returns false if the buffer is malformed or a handler returns false.
bool WalkChunks(const uint8_t* buf, size_t total, const std::unordered_map<ChunkId, ChunkHandler>& handlers);

} // namespace mve

#endif