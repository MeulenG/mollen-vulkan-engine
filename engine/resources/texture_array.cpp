#include "texture_array.h"
#include "buffer.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace mve {

TextureArray::TextureArray(Device& device,
                           uint32_t width, uint32_t height,
                           uint32_t slices, vk::Format format,
                           uint32_t mip_levels)
    : pm_device{device},
      pm_width{width}, pm_height{height},
      pm_slices{slices}, pm_format{format},
      pm_mip_levels{std::max(1u, mip_levels)} {

    CreateImage();
    CreateImageView();
    CreateSampler();
    // Start in TransferDst layout so the first UploadSlice call doesn't
    // need a layout transition; FinalizeForSampling flips it once at
    // the end.
    TransitionLayout(vk::ImageLayout::eUndefined,
                     vk::ImageLayout::eTransferDstOptimal);
    pm_layout_is_transfer_dst = true;
}

void TextureArray::CreateImage() {
    vk::ImageCreateInfo image_info{};
    image_info.imageType   = vk::ImageType::e2D;
    image_info.format      = pm_format;
    image_info.extent      = vk::Extent3D{pm_width, pm_height, 1};
    image_info.mipLevels   = pm_mip_levels;
    image_info.arrayLayers = pm_slices;
    image_info.samples     = vk::SampleCountFlagBits::e1;
    image_info.tiling      = vk::ImageTiling::eOptimal;
    image_info.usage       = vk::ImageUsageFlagBits::eTransferDst |
                             vk::ImageUsageFlagBits::eSampled;

    pm_image = pm_device.GetDevice().createImage(image_info);

    auto mem_reqs = pm_image.getMemoryRequirements();
    pm_memory = pm_device.GetDevice().allocateMemory({
        mem_reqs.size,
        pm_device.FindMemoryType(mem_reqs.memoryTypeBits,
                                  vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    pm_image.bindMemory(*pm_memory, 0);
}

void TextureArray::CreateImageView() {
    vk::ImageViewCreateInfo view_info{};
    view_info.image    = *pm_image;
    view_info.viewType = vk::ImageViewType::e2DArray;
    view_info.format   = pm_format;
    view_info.subresourceRange = {
        vk::ImageAspectFlagBits::eColor,
        0, pm_mip_levels,
        0, pm_slices,
    };
    pm_image_view = pm_device.GetDevice().createImageView(view_info);
}

void TextureArray::CreateSampler() {
    auto limits = pm_device.GetPhysicalDevice().getProperties().limits;
    float max_aniso = std::min(16.0f, limits.maxSamplerAnisotropy);

    vk::SamplerCreateInfo sampler_info{};
    sampler_info.magFilter        = vk::Filter::eLinear;
    sampler_info.minFilter        = vk::Filter::eLinear;
    sampler_info.addressModeU     = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeV     = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeW     = vk::SamplerAddressMode::eClampToEdge;
    sampler_info.anisotropyEnable = vk::True;
    sampler_info.maxAnisotropy    = max_aniso;
    sampler_info.borderColor      = vk::BorderColor::eIntOpaqueBlack;
    sampler_info.mipmapMode       = vk::SamplerMipmapMode::eLinear;
    sampler_info.maxLod           = static_cast<float>(pm_mip_levels - 1);
    pm_sampler = pm_device.GetDevice().createSampler(sampler_info);
}

void TextureArray::TransitionLayout(vk::ImageLayout old_layout,
                                    vk::ImageLayout new_layout) {
    auto cmd = pm_device.BeginSingleTimeCommands();

    vk::ImageMemoryBarrier2 barrier{};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = *pm_image;
    barrier.subresourceRange = {
        vk::ImageAspectFlagBits::eColor,
        0, pm_mip_levels,
        0, pm_slices,
    };

    if (old_layout == vk::ImageLayout::eUndefined &&
        new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.srcAccessMask = {};
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eTransfer;
        barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
    } else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
               new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eTransfer;
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eFragmentShader;
        barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
    }

    vk::DependencyInfo dep_info{};
    dep_info.setImageMemoryBarriers(barrier);
    cmd.pipelineBarrier2(dep_info);
    pm_device.EndSingleTimeCommands(std::move(cmd));
}

void TextureArray::FinalizeForSampling() {
    if (!pm_layout_is_transfer_dst) return;
    TransitionLayout(vk::ImageLayout::eTransferDstOptimal,
                     vk::ImageLayout::eShaderReadOnlyOptimal);
    pm_layout_is_transfer_dst = false;
}

bool TextureArray::UploadSliceFromBlp(uint32_t slice, const BlpTexture& blp) {
    if (slice >= pm_slices) return false;
    if (blp.format != pm_format) return false;

    // Pick the BLP mip whose width matches our slice width.
    // log2(blp.width / target_width). For wider BLPs this trims off
    // a higher-res mip; for narrower BLPs we'd be upscaling, which
    // we refuse for now (assigns slice 0 fallback elsewhere).
    if (blp.width < pm_width) return false;
    uint32_t skip_mips = 0;
    uint32_t w = blp.width;
    while (w > pm_width && skip_mips + 1 < blp.mip_count) {
        w >>= 1;
        skip_mips++;
    }
    if (w != pm_width) return false;

    // How many mips of the array can we feed from the BLP? Take the
    // min of remaining BLP mips and our array's mip count.
    uint32_t feedable = std::min<uint32_t>(blp.mip_count - skip_mips,
                                            pm_mip_levels);
    if (feedable == 0) return false;

    // Build one staging buffer holding the chosen mips contiguously.
    size_t total = 0;
    for (uint32_t m = 0; m < feedable; m++) {
        total += blp.mip_data[skip_mips + m].size();
    }
    if (total == 0) return false;

    Buffer staging{pm_device, total,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent};
    auto* mapped = static_cast<uint8_t*>(staging.Map());
    size_t off = 0;
    for (uint32_t m = 0; m < feedable; m++) {
        const auto& src = blp.mip_data[skip_mips + m];
        std::memcpy(mapped + off, src.data(), src.size());
        off += src.size();
    }
    staging.Unmap();

    auto cmd = pm_device.BeginSingleTimeCommands();
    off = 0;
    for (uint32_t m = 0; m < feedable; m++) {
        uint32_t mip_w = std::max(1u, pm_width  >> m);
        uint32_t mip_h = std::max(1u, pm_height >> m);

        vk::BufferImageCopy region{};
        region.bufferOffset = off;
        region.imageSubresource = {
            vk::ImageAspectFlagBits::eColor,
            m, slice, 1,
        };
        region.imageExtent = vk::Extent3D{mip_w, mip_h, 1};
        cmd.copyBufferToImage(*staging.GetBuffer(), *pm_image,
                              vk::ImageLayout::eTransferDstOptimal, region);
        off += blp.mip_data[skip_mips + m].size();
    }
    pm_device.EndSingleTimeCommands(std::move(cmd));
    return true;
}

void TextureArray::UploadSlicePixels(uint32_t slice, uint32_t mip,
                                     const void* pixels, size_t byte_count) {
    if (slice >= pm_slices) return;
    if (mip >= pm_mip_levels) return;

    Buffer staging{pm_device, byte_count,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent};
    staging.Write(pixels, byte_count);

    uint32_t mip_w = std::max(1u, pm_width  >> mip);
    uint32_t mip_h = std::max(1u, pm_height >> mip);
    (void)byte_count;  // Caller is responsible for the correct mip byte size.

    auto cmd = pm_device.BeginSingleTimeCommands();
    vk::BufferImageCopy region{};
    region.imageSubresource = {
        vk::ImageAspectFlagBits::eColor,
        mip, slice, 1,
    };
    region.imageExtent = vk::Extent3D{mip_w, mip_h, 1};
    cmd.copyBufferToImage(*staging.GetBuffer(), *pm_image,
                          vk::ImageLayout::eTransferDstOptimal, region);
    pm_device.EndSingleTimeCommands(std::move(cmd));
}

} // namespace mve
