#ifndef MVE_TEXTURE_ARRAY_H
#define MVE_TEXTURE_ARRAY_H

#include "../core/device.h"
#include "../formats/blp_loader.h"

#include <cstdint>
#include <vector>

namespace mve {

// 2D texture array. All slices share width, height, format, and mip
// levels. Used by terrain for the per-tile diffuse atlas (one slice per
// unique tile texture) and the per-chunk alpha-blend atlas (one slice
// per MCNK chunk).
//
// Two upload paths:
//   - UploadSlice(slice, blp, src_mip): pulls one mip-aligned slab out
//     of a parsed BLP and writes it into the array's slice.
//   - UploadSlicePixels(slice, mip, pixels, byte_count): writes raw
//     bytes into a single mip of one slice. Used by the alpha-map
//     atlas which builds its pixels in-engine, not from a BLP.
class TextureArray {
public:
    TextureArray(Device& device,
                 uint32_t width, uint32_t height,
                 uint32_t slices, vk::Format format,
                 uint32_t mip_levels);

    TextureArray(const TextureArray&) = delete;
    TextureArray& operator=(const TextureArray&) = delete;
    TextureArray(TextureArray&&) = default;
    TextureArray& operator=(TextureArray&&) = default;

    // Picks the BLP mip that matches the array's slice width (so a
    // 1024-texel BLP feeding a 256-slice picks mip 2). Uploads that mip
    // and the smaller ones into the same slice. Slots that aren't
    // covered by the BLP's mip chain are left undefined (the sampler's
    // maxLod clamp prevents them being read).
    //
    // Returns true on success, false on a size or format mismatch.
    bool UploadSliceFromBlp(uint32_t slice, const BlpTexture& blp);

    // Raw upload of one mip of one slice. The caller is responsible for
    // computing the correct byte count for the format (e.g. BC3 = 16
    // bytes per 4x4 block; RGBA8 = 4 bytes per texel).
    void UploadSlicePixels(uint32_t slice, uint32_t mip,
                           const void* pixels, size_t byte_count);

    // Transition all slices/mips from TransferDst -> ShaderReadOnly.
    // Must be called once after all uploads are done and before the
    // array is bound for sampling.
    void FinalizeForSampling();

    vk::Format Format() const { return pm_format; }
    uint32_t Width() const { return pm_width; }
    uint32_t Height() const { return pm_height; }
    uint32_t Slices() const { return pm_slices; }
    uint32_t MipLevels() const { return pm_mip_levels; }

    const vk::raii::ImageView& GetImageView() const { return pm_image_view; }
    const vk::raii::Sampler& GetSampler() const { return pm_sampler; }

    vk::DescriptorImageInfo DescriptorInfo() const {
        return {*pm_sampler, *pm_image_view,
                vk::ImageLayout::eShaderReadOnlyOptimal};
    }

private:
    void CreateImage();
    void CreateImageView();
    void CreateSampler();
    void TransitionLayout(vk::ImageLayout old_layout,
                          vk::ImageLayout new_layout);

    Device& pm_device;
    uint32_t pm_width;
    uint32_t pm_height;
    uint32_t pm_slices;
    vk::Format pm_format;
    uint32_t pm_mip_levels;
    bool pm_layout_is_transfer_dst = true;  // post-construction

    vk::raii::Image       pm_image{nullptr};
    vk::raii::DeviceMemory pm_memory{nullptr};
    vk::raii::ImageView   pm_image_view{nullptr};
    vk::raii::Sampler     pm_sampler{nullptr};
};

} // namespace mve

#endif // MVE_TEXTURE_ARRAY_H
