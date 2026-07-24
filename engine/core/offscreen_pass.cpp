#include "offscreen_pass.h"

#include "../resources/buffer.h"

// stb_image_write provides PNG/JPG/BMP/TGA encoders as a single header.
// Define STB_IMAGE_WRITE_IMPLEMENTATION exactly once across the whole
// translation unit graph - this is that spot.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb_image_write.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace mve {

OffscreenPass::OffscreenPass(Device& device, uint32_t width, uint32_t height,
                             vk::Format color_format, vk::Format depth_format)
    : device_{device}, width_{width}, height_{height},
      color_format_{color_format}, depth_format_{depth_format} {
    createResources();
}

void OffscreenPass::createResources() {
    // --- Multisample color (render target) ---
    // 4x MSAA. Used only as a color attachment - we never sample it
    // directly. The GPU resolves the 4 samples per pixel into the
    // single-sample color_image_ below at EndRendering time.
    vk::ImageCreateInfo msaa_color_info{};
    msaa_color_info.imageType = vk::ImageType::e2D;
    msaa_color_info.format = color_format_;
    msaa_color_info.extent = vk::Extent3D{width_, height_, 1};
    msaa_color_info.mipLevels = 1;
    msaa_color_info.arrayLayers = 1;
    msaa_color_info.samples = kSampleCount;
    msaa_color_info.tiling = vk::ImageTiling::eOptimal;
    msaa_color_info.usage = vk::ImageUsageFlagBits::eColorAttachment;

    msaa_color_image_ = device_.GetDevice().createImage(msaa_color_info);
    auto msaa_reqs = msaa_color_image_.getMemoryRequirements();
    msaa_color_memory_ = device_.GetDevice().allocateMemory({
        msaa_reqs.size,
        device_.FindMemoryType(msaa_reqs.memoryTypeBits,
                                vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    msaa_color_image_.bindMemory(*msaa_color_memory_, 0);

    vk::ImageViewCreateInfo msaa_view_info{};
    msaa_view_info.image = *msaa_color_image_;
    msaa_view_info.viewType = vk::ImageViewType::e2D;
    msaa_view_info.format = color_format_;
    msaa_view_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    msaa_color_view_ = device_.GetDevice().createImageView(msaa_view_info);

    // --- Single-sample resolve target (ImGui samples this) ---
    vk::ImageCreateInfo color_info{};
    color_info.imageType = vk::ImageType::e2D;
    color_info.format = color_format_;
    color_info.extent = vk::Extent3D{width_, height_, 1};
    color_info.mipLevels = 1;
    color_info.arrayLayers = 1;
    color_info.samples = vk::SampleCountFlagBits::e1;
    color_info.tiling = vk::ImageTiling::eOptimal;
    // COLOR_ATTACHMENT: we bind it as the resolve target in the render
    // pass (Vulkan classifies resolve attachments as a color attachment
    // usage). SAMPLED: ImGui reads it as a texture. TRANSFER_SRC:
    // SaveColorToPng copies the bytes out for PNG dumps via
    // vkCmdCopyImageToBuffer.
    color_info.usage = vk::ImageUsageFlagBits::eColorAttachment
                     | vk::ImageUsageFlagBits::eSampled
                     | vk::ImageUsageFlagBits::eTransferSrc;

    color_image_ = device_.GetDevice().createImage(color_info);
    auto color_reqs = color_image_.getMemoryRequirements();
    color_memory_ = device_.GetDevice().allocateMemory({
        color_reqs.size,
        device_.FindMemoryType(color_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    color_image_.bindMemory(*color_memory_, 0);

    vk::ImageViewCreateInfo color_view_info{};
    color_view_info.image = *color_image_;
    color_view_info.viewType = vk::ImageViewType::e2D;
    color_view_info.format = color_format_;
    color_view_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    color_view_ = device_.GetDevice().createImageView(color_view_info);

    // --- Multisample depth attachment ---
    // 4x to match the MSAA color attachment (Vulkan requires all
    // attachments in a render pass to share the same sample count).
    // No resolve: depth is consumed for early-Z + depth-test then
    // discarded; we never sample it.
    vk::ImageCreateInfo depth_info{};
    depth_info.imageType = vk::ImageType::e2D;
    depth_info.format = depth_format_;
    depth_info.extent = vk::Extent3D{width_, height_, 1};
    depth_info.mipLevels = 1;
    depth_info.arrayLayers = 1;
    depth_info.samples = kSampleCount;
    depth_info.tiling = vk::ImageTiling::eOptimal;
    depth_info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

    depth_image_ = device_.GetDevice().createImage(depth_info);
    auto depth_reqs = depth_image_.getMemoryRequirements();
    depth_memory_ = device_.GetDevice().allocateMemory({
        depth_reqs.size,
        device_.FindMemoryType(depth_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    depth_image_.bindMemory(*depth_memory_, 0);

    vk::ImageViewCreateInfo depth_view_info{};
    depth_view_info.image = *depth_image_;
    depth_view_info.viewType = vk::ImageViewType::e2D;
    depth_view_info.format = depth_format_;
    depth_view_info.subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};
    depth_view_ = device_.GetDevice().createImageView(depth_view_info);

    // Sampler to read color image
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.magFilter = vk::Filter::eLinear;
    sampler_info.minFilter = vk::Filter::eLinear;
    sampler_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    sampler_ = device_.GetDevice().createSampler(sampler_info);
}

void OffscreenPass::BeginRendering(const vk::raii::CommandBuffer& cmd) {
    // Transition the MSAA color image to ColorAttachmentOptimal - it's
    // the render target the GPU writes 4 samples per pixel into.
    transitionImage(cmd, *msaa_color_image_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal);

    // Transition the single-sample resolve target to ColorAttachmentOptimal
    // too. Vulkan classifies resolve attachments as a color-attachment
    // usage during the render pass, so the layout must match.
    transitionImage(cmd, *color_image_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal);

    transitionImage(cmd, *depth_image_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::ImageAspectFlagBits::eDepth);

    vk::RenderingAttachmentInfo color_attachment{};
    color_attachment.imageView   = *msaa_color_view_;
    color_attachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    // Resolve: at end-of-render-pass, the GPU averages the 4 MSAA
    // samples per pixel and writes the result to color_view_. After
    // this, ImGui samples color_view_ for the viewport panel.
    color_attachment.resolveMode        = vk::ResolveModeFlagBits::eAverage;
    color_attachment.resolveImageView   = *color_view_;
    color_attachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    color_attachment.loadOp  = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    color_attachment.clearValue.color = vk::ClearColorValue{
        std::array{0.02f, 0.02f, 0.02f, 1.0f}};

    vk::RenderingAttachmentInfo depth_attachment{};
    depth_attachment.imageView = *depth_view_;
    depth_attachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
    depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depth_attachment.clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderingInfo rendering_info{};
    rendering_info.renderArea = vk::Rect2D{vk::Offset2D{0, 0}, vk::Extent2D{width_, height_}};
    rendering_info.layerCount = 1;
    rendering_info.setColorAttachments(color_attachment);
    rendering_info.pDepthAttachment = &depth_attachment;

    cmd.beginRendering(rendering_info);

    vk::Viewport viewport{0.0f, 0.0f, float(width_), float(height_), 0.0f, 1.0f};
    cmd.setViewport(0, viewport);

    vk::Rect2D scissor{vk::Offset2D{0, 0}, vk::Extent2D{width_, height_}};
    cmd.setScissor(0, scissor);
}

void OffscreenPass::EndRendering(const vk::raii::CommandBuffer& cmd) {
    cmd.endRendering();

    // Transition the resolve target to ShaderReadOnly so ImGui can
    // sample it. The MSAA color image stays in ColorAttachmentOptimal -
    // we never sample it directly, and the next frame's BeginRendering
    // transitions it from Undefined again (a no-op since loadOp = Clear
    // discards prior contents anyway).
    transitionImage(cmd, *color_image_,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);
}

void OffscreenPass::Resize(uint32_t width, uint32_t height) {
    if (width == width_ && height == height_) return;
    if (width == 0 || height == 0) return;

    device_.GetDevice().waitIdle();

    width_ = width;
    height_ = height;

    // Destroy old resources (RAII handles this when we reassign).
    // Include the MSAA color image + memory + view introduced in 2A.
    msaa_color_view_   = nullptr;
    msaa_color_image_  = nullptr;
    msaa_color_memory_ = nullptr;
    color_view_   = nullptr;
    color_image_  = nullptr;
    color_memory_ = nullptr;
    depth_view_   = nullptr;
    depth_image_  = nullptr;
    depth_memory_ = nullptr;

    createResources();
}

void OffscreenPass::transitionImage(const vk::raii::CommandBuffer& cmd, vk::Image image,
                                     vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                                     vk::ImageAspectFlags aspect) {
    vk::ImageMemoryBarrier2 barrier{};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange = {aspect, 0, 1, 0, 1};

    if (old_layout == vk::ImageLayout::eUndefined &&
        new_layout == vk::ImageLayout::eColorAttachmentOptimal) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    } else if (old_layout == vk::ImageLayout::eColorAttachmentOptimal &&
               new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
        barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
    } else if (old_layout == vk::ImageLayout::eUndefined &&
               new_layout == vk::ImageLayout::eDepthAttachmentOptimal) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
        barrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
    }

    vk::DependencyInfo dep_info{};
    dep_info.setImageMemoryBarriers(barrier);
    cmd.pipelineBarrier2(dep_info);
}

bool OffscreenPass::SaveColorToPng(const std::string& path) {
    // 1. Allocate a host-visible staging buffer sized for one packed
    //    RGBA8 image of the offscreen color attachment's dimensions.
    //    Image is in B8G8R8A8_SRGB on the typical swapchain; the byte
    //    layout in memory is { B, G, R, A } per pixel.
    const vk::DeviceSize bytes_per_pixel = 4;
    const vk::DeviceSize image_bytes =
        static_cast<vk::DeviceSize>(width_) * height_ * bytes_per_pixel;

    Buffer staging{device_, image_bytes,
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent};

    // 2. Record + submit a one-shot command buffer that:
    //    a. Transitions color image from ShaderReadOnly -> TransferSrc
    //    b. Copies the whole image to the staging buffer
    //    c. Transitions back to ShaderReadOnly so the next frame's
    //       ImGui sampling still works
    auto cmd = device_.BeginSingleTimeCommands();

    // a. Layout: ShaderReadOnly -> TransferSrcOptimal
    {
        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = *color_image_;
        barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eFragmentShader;
        barrier.srcAccessMask = vk::AccessFlagBits2::eShaderRead;
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eTransfer;
        barrier.dstAccessMask = vk::AccessFlagBits2::eTransferRead;
        vk::DependencyInfo dep_info{};
        dep_info.setImageMemoryBarriers(barrier);
        cmd.pipelineBarrier2(dep_info);
    }

    // b. Copy color image -> staging buffer (tightly packed, no row pitch padding)
    {
        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;        // 0 = tightly packed = width
        region.bufferImageHeight = 0;      // 0 = tightly packed = height
        region.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
        region.imageOffset = vk::Offset3D{0, 0, 0};
        region.imageExtent = vk::Extent3D{width_, height_, 1};
        cmd.copyImageToBuffer(*color_image_,
                              vk::ImageLayout::eTransferSrcOptimal,
                              *staging.GetBuffer(),
                              region);
    }

    // c. Layout: TransferSrcOptimal -> ShaderReadOnlyOptimal
    {
        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = *color_image_;
        barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eTransfer;
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eFragmentShader;
        barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
        vk::DependencyInfo dep_info{};
        dep_info.setImageMemoryBarriers(barrier);
        cmd.pipelineBarrier2(dep_info);
    }

    device_.EndSingleTimeCommands(std::move(cmd));

    // 3. Read the bytes back. BGRA on disk for B8G8R8A8 formats; swap
    //    bytes 0 and 2 per pixel so stb_image_write writes RGBA.
    std::vector<uint8_t> pixels(image_bytes);
    void* mapped = staging.Map();
    std::memcpy(pixels.data(), mapped, image_bytes);
    staging.Unmap();

    for (vk::DeviceSize i = 0; i + 4 <= image_bytes; i += 4) {
        std::swap(pixels[i + 0], pixels[i + 2]);   // B <-> R
        // alpha (i + 3) and green (i + 1) stay put
    }

    // 4. Write PNG. stride_in_bytes = width * 4 because we copied tightly.
    int ok = stbi_write_png(
        path.c_str(),
        static_cast<int>(width_),
        static_cast<int>(height_),
        4,
        pixels.data(),
        static_cast<int>(width_ * 4));

    if (!ok) {
        std::fprintf(stderr,
            "OffscreenPass::SaveColorToPng failed to write %s\n",
            path.c_str());
        return false;
    }
    std::fprintf(stderr,
        "Saved screenshot: %s (%ux%u)\n",
        path.c_str(), width_, height_);
    return true;
}

} // namespace mve
