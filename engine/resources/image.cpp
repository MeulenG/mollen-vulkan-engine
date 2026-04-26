#include "image.h"
#include "buffer.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace mve {

Image::Image(Device& device, uint32_t width, uint32_t height, const uint8_t* pixels)
    : device_{device} {
    createImage(width, height, vk::Format::eR8G8B8A8Srgb);
    createImageView(vk::Format::eR8G8B8A8Srgb);
    createSampler();
    uploadPixels(pixels, width, height);
}

Image::Image(Device& device, const BlpTexture& blp)
    : device_{device} {
    createImage(blp.width, blp.height, blp.format, blp.mip_count);
    createImageView(blp.format, blp.mip_count);
    createSampler(blp.mip_count);

    if (blp.compressed) {
        uploadCompressed(blp);
    } else {
        uploadPixels(blp.mip_data[0].data(), blp.width, blp.height);
    }
}

void Image::createImage(uint32_t width, uint32_t height, vk::Format format, uint32_t mip_levels) {
    vk::ImageCreateInfo image_info{};
    image_info.imageType = vk::ImageType::e2D;
    image_info.format = format;
    image_info.extent = vk::Extent3D{width, height, 1};
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.tiling = vk::ImageTiling::eOptimal;
    image_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;

    image_ = device_.GetDevice().createImage(image_info);

    auto mem_reqs = image_.getMemoryRequirements();
    memory_ = device_.GetDevice().allocateMemory({
        mem_reqs.size,
        device_.FindMemoryType(mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    image_.bindMemory(*memory_, 0);
}

void Image::createImageView(vk::Format format, uint32_t mip_levels) {
    vk::ImageViewCreateInfo view_info{};
    view_info.image = *image_;
    view_info.viewType = vk::ImageViewType::e2D;
    view_info.format = format;
    view_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, mip_levels, 0, 1};

    image_view_ = device_.GetDevice().createImageView(view_info);
}

void Image::createSampler(uint32_t mip_levels) {
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.magFilter = vk::Filter::eLinear;
    sampler_info.minFilter = vk::Filter::eLinear;
    sampler_info.addressModeU = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeV = vk::SamplerAddressMode::eRepeat;
    sampler_info.addressModeW = vk::SamplerAddressMode::eRepeat;
    sampler_info.anisotropyEnable = vk::False;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.borderColor = vk::BorderColor::eIntOpaqueBlack;
    sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler_info.maxLod = static_cast<float>(mip_levels - 1);

    sampler_ = device_.GetDevice().createSampler(sampler_info);
}

void Image::uploadPixels(const uint8_t* pixels, uint32_t width, uint32_t height) {
    vk::DeviceSize image_size = width * height * 4;

    Buffer staging{device_, image_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};
    staging.Write(pixels, image_size);

    transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    auto cmd = device_.BeginSingleTimeCommands();
    vk::BufferImageCopy region{};
    region.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
    region.imageExtent = vk::Extent3D{width, height, 1};
    cmd.copyBufferToImage(*staging.GetBuffer(), *image_, vk::ImageLayout::eTransferDstOptimal, region);
    device_.EndSingleTimeCommands(std::move(cmd));

    transitionLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Image::uploadCompressed(const BlpTexture& blp) {
    // For DXT/BC compressed textures, we upload each mip level as raw compressed blocks.
    // The GPU decompresses them in hardware during texture sampling.

    // Calculate total staging buffer size
    vk::DeviceSize total_size = 0;
    for (const auto& mip : blp.mip_data) {
        total_size += mip.size();
    }

    Buffer staging{device_, total_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    // Copy all mip levels into staging buffer sequentially
    auto* mapped = static_cast<uint8_t*>(staging.Map());
    vk::DeviceSize staging_offset = 0;
    for (const auto& mip : blp.mip_data) {
        std::memcpy(mapped + staging_offset, mip.data(), mip.size());
        staging_offset += mip.size();
    }
    staging.Unmap();

    transitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, blp.mip_count);

    auto cmd = device_.BeginSingleTimeCommands();

    // Issue one copy command per mip level
    staging_offset = 0;
    for (uint32_t m = 0; m < blp.mip_count; m++) {
        uint32_t mip_w = std::max(1u, blp.width >> m);
        uint32_t mip_h = std::max(1u, blp.height >> m);

        // BC formats work in 4x4 blocks — minimum dimension is 4
        // But Vulkan handles this internally; we just specify the actual pixel dimensions

        vk::BufferImageCopy region{};
        region.bufferOffset = staging_offset;
        region.imageSubresource = {vk::ImageAspectFlagBits::eColor, m, 0, 1};
        region.imageExtent = vk::Extent3D{mip_w, mip_h, 1};

        cmd.copyBufferToImage(*staging.GetBuffer(), *image_, vk::ImageLayout::eTransferDstOptimal, region);

        staging_offset += blp.mip_data[m].size();
    }

    device_.EndSingleTimeCommands(std::move(cmd));

    transitionLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, blp.mip_count);
}

void Image::transitionLayout(vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t mip_levels) {
    auto cmd = device_.BeginSingleTimeCommands();

    vk::ImageMemoryBarrier2 barrier{};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = *image_;
    barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, mip_levels, 0, 1};

    if (old_layout == vk::ImageLayout::eUndefined &&
        new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.srcAccessMask = {};
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
        barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
    } else if (old_layout == vk::ImageLayout::eTransferDstOptimal &&
               new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
        barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
    }

    vk::DependencyInfo dep_info{};
    dep_info.setImageMemoryBarriers(barrier);
    cmd.pipelineBarrier2(dep_info);

    device_.EndSingleTimeCommands(std::move(cmd));
}

Image Image::CreateCheckerboard(Device& device, uint32_t size, uint32_t cell_size) {
    std::vector<uint8_t> pixels(size * size * 4);

    for (uint32_t y = 0; y < size; y++) {
        for (uint32_t x = 0; x < size; x++) {
            bool white = ((x / cell_size) + (y / cell_size)) % 2 == 0;
            uint8_t val = white ? 255 : 180;

            uint32_t idx = (y * size + x) * 4;
            pixels[idx + 0] = val;
            pixels[idx + 1] = val;
            pixels[idx + 2] = val;
            pixels[idx + 3] = 255;
        }
    }

    return Image{device, size, size, pixels.data()};
}

} // namespace mve
