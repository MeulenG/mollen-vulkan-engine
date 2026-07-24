#include "blp_loader.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace mve {

static constexpr uint32_t BLP2_MAGIC = 0x32504C42; // "BLP2"

BlpTexture BlpLoader::load(const uint8_t* data, uint32_t size) {
    if (size < sizeof(BlpHeader)) {
        throw std::runtime_error("BLP file too small for header");
    }

    BlpHeader header;
    std::memcpy(&header, data, sizeof(BlpHeader));

    if (header.magic != BLP2_MAGIC) {
        throw std::runtime_error("Invalid BLP magic number");
    }

    if (header.compression == 2) {
        return loadDxt(header, data);
    } else if (header.compression == 1) {
        return loadPalettized(header, data);
    } else {
        throw std::runtime_error("Unsupported BLP compression type: " + std::to_string(header.compression));
    }
}

BlpTexture BlpLoader::LoadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open BLP file: " + path);
    }

    auto file_size = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> buffer(static_cast<size_t>(file_size));
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);

    return load(buffer.data(), static_cast<uint32_t>(buffer.size()));
}

BlpTexture BlpLoader::loadDxt(const BlpHeader& header, const uint8_t* data) {
    BlpTexture tex{};
    tex.width = header.width;
    tex.height = header.height;
    tex.compressed = true;

    // Determine DXT format from alpha_type:
    //   alpha_type 0 → DXT1 (BC1) - 8 bytes per 4x4 block, 0 or 1-bit alpha
    //   alpha_type 1 → DXT3 (BC2) - 16 bytes per 4x4 block, explicit 4-bit alpha
    //   alpha_type 7 → DXT5 (BC3) - 16 bytes per 4x4 block, interpolated alpha
    //
    // BC1 block = 8 bytes:  2 RGB565 colors + 4x4 2-bit lookup table
    // BC2 block = 16 bytes: 4x4 4-bit alpha values + BC1 color block
    // BC3 block = 16 bytes: 2 alpha values + 4x4 3-bit alpha lookup + BC1 color block
    // sRGB variants. BLPs store color data in sRGB space (the artists
    // painted in sRGB, the BLPs preserve those values), but our final
    // swapchain target is also sRGB. Loading the texture as a UNorm
    // format causes the GPU to sample sRGB data WITHOUT decoding it
    // to linear, so all lighting math (NdotL multiplies, mixes, etc.)
    // runs in nonlinear space. Then the sRGB swapchain output applies
    // gamma encoding on top of that, double-encoding everything and
    // producing muddy / dark midtones.
    //
    // The correct chain is:
    //   sample (sRGB texel -> linear via hardware)
    //   lighting math in linear space
    //   write (linear result -> sRGB via hardware on the target)
    //
    // The Vulkan *_SrgbBlock formats trigger the hardware sRGB->linear
    // decode on sample for free. No shader change is needed because
    // the conversion happens before the texel reaches the shader.
    switch (header.alpha_type) {
        case 0:
            tex.format = (header.alpha_depth > 0)
                ? vk::Format::eBc1RgbaSrgbBlock   // DXT1 with alpha
                : vk::Format::eBc1RgbSrgbBlock;   // DXT1 no alpha
            break;
        case 1:
            tex.format = vk::Format::eBc2SrgbBlock; // DXT3
            break;
        case 7:
            tex.format = vk::Format::eBc3SrgbBlock; // DXT5
            break;
        default:
            throw std::runtime_error("Unknown BLP DXT alpha_type: " + std::to_string(header.alpha_type));
    }

    // Count valid mip levels
    tex.mip_count = 0;
    for (int i = 0; i < 16; i++) {
        if (header.mip_offsets[i] == 0 || header.mip_sizes[i] == 0) break;
        tex.mip_count++;
    }

    if (tex.mip_count == 0) {
        throw std::runtime_error("BLP has no mip levels");
    }

    // Copy each mip level's raw compressed data
    tex.mip_data.resize(tex.mip_count);
    for (uint32_t i = 0; i < tex.mip_count; i++) {
        uint32_t offset = header.mip_offsets[i];
        uint32_t length = header.mip_sizes[i];

        tex.mip_data[i].resize(length);
        std::memcpy(tex.mip_data[i].data(), data + offset, length);
    }

    return tex;
}

BlpTexture BlpLoader::loadPalettized(const BlpHeader& header, const uint8_t* data) {
    BlpTexture tex{};
    tex.width = header.width;
    tex.height = header.height;
    tex.compressed = false;
    // sRGB on sample - see DXT path above for the gamma-chain
    // reasoning. Palettized BLPs are uncommon for diffuse art (mostly
    // UI icons and minimap pieces) but the same sRGB->linear-on-
    // sample rule applies.
    tex.format = vk::Format::eR8G8B8A8Srgb;

    // Palette: 256 BGRA entries (1024 bytes) immediately after the header
    struct BgrxColor { uint8_t b, g, r, x; };
    const BgrxColor* palette = reinterpret_cast<const BgrxColor*>(data + sizeof(BlpHeader));

    // Count mip levels
    tex.mip_count = 0;
    for (int i = 0; i < 16; i++) {
        if (header.mip_offsets[i] == 0 || header.mip_sizes[i] == 0) break;
        tex.mip_count++;
    }

    if (tex.mip_count == 0) {
        throw std::runtime_error("BLP has no mip levels");
    }

    tex.mip_data.resize(tex.mip_count);

    for (uint32_t m = 0; m < tex.mip_count; m++) {
        uint32_t mip_w = std::max(1u, header.width >> m);
        uint32_t mip_h = std::max(1u, header.height >> m);
        uint32_t pixel_count = mip_w * mip_h;

        // The mip data contains palette indices (1 byte per pixel)
        const uint8_t* indices = data + header.mip_offsets[m];

        // Alpha data follows the indices (if alpha_depth > 0)
        const uint8_t* alpha_data = nullptr;
        if (header.alpha_depth > 0) {
            alpha_data = indices + pixel_count;
        }

        // Convert to RGBA8
        tex.mip_data[m].resize(pixel_count * 4);
        uint8_t* out = tex.mip_data[m].data();

        for (uint32_t i = 0; i < pixel_count; i++) {
            const BgrxColor& c = palette[indices[i]];
            out[i * 4 + 0] = c.r;
            out[i * 4 + 1] = c.g;
            out[i * 4 + 2] = c.b;

            // Alpha handling depends on alpha_depth
            if (header.alpha_depth == 0) {
                out[i * 4 + 3] = 255;
            } else if (header.alpha_depth == 1) {
                // 1 bit per pixel, packed 8 per byte
                uint8_t bit = (alpha_data[i / 8] >> (i % 8)) & 1;
                out[i * 4 + 3] = bit ? 255 : 0;
            } else if (header.alpha_depth == 4) {
                // 4 bits per pixel, 2 per byte
                uint8_t nibble = (i % 2 == 0)
                    ? (alpha_data[i / 2] & 0x0F)
                    : (alpha_data[i / 2] >> 4);
                out[i * 4 + 3] = nibble * 17; // scale 0-15 to 0-255
            } else if (header.alpha_depth == 8) {
                out[i * 4 + 3] = alpha_data[i];
            }
        }
    }

    return tex;
}

} // namespace mve
