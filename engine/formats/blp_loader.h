#ifndef MVE_BLP_LOADER_H
#define MVE_BLP_LOADER_H

#include "../core/device.h"

#include <cstdint>
#include <string>
#include <vector>

namespace mve {

// BLP2 file format — Blizzard's texture format used in WoW.
//
// Two main compression modes:
// 1. Palettized: 256-color BGRA palette + 8-bit indices per pixel
// 2. DXT compressed: S3TC block compression (DXT1, DXT3, DXT5)
//    - DXT1 (BC1): 4:1 compression, 1-bit alpha
//    - DXT3 (BC2): 4:1 compression, explicit 4-bit alpha
//    - DXT5 (BC3): 4:1 compression, interpolated 8-bit alpha
//
// WotLK primarily uses DXT compression. Vulkan supports BC1/BC2/BC3
// natively, so we can upload compressed data directly to GPU.

struct BlpHeader {
    uint32_t magic;           // "BLP2" = 0x32504C42
    uint32_t type;            // 0 = JPEG (unused in WotLK), 1 = direct/DXT
    uint8_t  compression;     // 1 = palettized, 2 = DXT, 3 = uncompressed
    uint8_t  alpha_depth;     // 0, 1, 4, or 8 bits of alpha
    uint8_t  alpha_type;      // 0 = DXT1, 1 = DXT3, 7 = DXT5 (when compression=2)
    uint8_t  has_mips;        // 0 or 1
    uint32_t width;
    uint32_t height;
    uint32_t mip_offsets[16]; // file offsets to each mip level
    uint32_t mip_sizes[16];   // byte sizes of each mip level
};

static_assert(sizeof(BlpHeader) == 148, "BLP header must be 148 bytes");

struct BlpTexture {
    uint32_t width;
    uint32_t height;
    vk::Format format;        // Vulkan format (BC1, BC2, BC3, or R8G8B8A8)
    uint32_t mip_count;
    bool compressed;           // true = DXT/BC, false = uncompressed RGBA

    // For compressed: raw DXT blocks per mip level
    // For uncompressed: RGBA8 pixel data per mip level
    std::vector<std::vector<uint8_t>> mip_data;
};

class BlpLoader {
public:
    // Parse a BLP file from raw bytes (e.g., extracted from MPQ)
    static BlpTexture load(const uint8_t* data, uint32_t size);

    // Parse a BLP file from disk
    static BlpTexture loadFile(const std::string& path);

private:
    static BlpTexture loadDxt(const BlpHeader& header, const uint8_t* data);
    static BlpTexture loadPalettized(const BlpHeader& header, const uint8_t* data);
};

} // namespace mve

#endif // MVE_BLP_LOADER_H
