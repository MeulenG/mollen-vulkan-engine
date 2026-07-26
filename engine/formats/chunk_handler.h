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
    // ADT top-level chunks
    MHDR,
    MTEX,
    MMDX,
    MMID,
    MWID,
    MDDF,
    MH2O,
    MCNK,
    // MCNK sub-chunks (offset-addressed from the MCNK header, not walked;
    // identified via IdentifyChunk when validating a sub-chunk tag).
    MCVT,
    MCNR,
    MCCV,
    MCLY,
    MCAL,
};

// Identify a chunk from its 4 on-disk id bytes (stored reversed, e.g.
// "MVER" appears as 'R','E','V','M'). Returns ChunkId::Unknown for
// unrecognized tags.
ChunkId IdentifyChunk(const uint8_t* id_bytes);

using ChunkHandler = std::function<bool(const uint8_t* data, uint32_t size)>;

// Walk a flat WoW chunk stream. Each registered handler is called for its
// matching ChunkId. Unknown chunk IDs are silently skipped.
// Returns false if the buffer is malformed or a handler returns false.
bool WalkChunks(const uint8_t* buf, size_t total, const std::unordered_map<ChunkId, ChunkHandler>& handlers);

} // namespace mve

#endif
