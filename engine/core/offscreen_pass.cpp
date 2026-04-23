#include "offscreen_pass.h"

#include <stdexcept>

namespace mve {

OffscreenPass::OffscreenPass(Device& device, uint32_t width, uint32_t height,
                             vk::Format color_format, vk::Format depth_format)
    : device_{device}, width_{width}, height_{height},
      color_format_{color_format}, depth_format_{depth_format} {
    createResources();
}

void OffscreenPass::createResources() {
    vk::ImageCreateInfo color_info{};
    color_info.imageType = vk::ImageType::e2D;
    color_info.format = color_format_;
    color_info.extent = vk::Extent3D{width_, height_, 1};
    color_info.mipLevels = 1;
    color_info.arrayLayers = 1;
    color_info.samples = vk::SampleCountFlagBits::e1;
    color_info.tiling = vk::ImageTiling::eOptimal;
    // COLOR_ATTACHMENT: we render to it. SAMPLED: ImGui reads it as a texture.
    color_info.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

    color_image_ = device_.device().createImage(color_info);
    auto color_reqs = color_image_.getMemoryRequirements();
    color_memory_ = device_.device().allocateMemory({
        color_reqs.size,
        device_.findMemoryType(color_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    color_image_.bindMemory(*color_memory_, 0);

    vk::ImageViewCreateInfo color_view_info{};
    color_view_info.image = *color_image_;
    color_view_info.viewType = vk::ImageViewType::e2D;
    color_view_info.format = color_format_;
    color_view_info.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    color_view_ = device_.device().createImageView(color_view_info);

    vk::ImageCreateInfo depth_info{};
    depth_info.imageType = vk::ImageType::e2D;
    depth_info.format = depth_format_;
    depth_info.extent = vk::Extent3D{width_, height_, 1};
    depth_info.mipLevels = 1;
    depth_info.arrayLayers = 1;
    depth_info.samples = vk::SampleCountFlagBits::e1;
    depth_info.tiling = vk::ImageTiling::eOptimal;
    depth_info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

    depth_image_ = device_.device().createImage(depth_info);
    auto depth_reqs = depth_image_.getMemoryRequirements();
    depth_memory_ = device_.device().allocateMemory({
        depth_reqs.size,
        device_.findMemoryType(depth_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    depth_image_.bindMemory(*depth_memory_, 0);

    vk::ImageViewCreateInfo depth_view_info{};
    depth_view_info.image = *depth_image_;
    depth_view_info.viewType = vk::ImageViewType::e2D;
    depth_view_info.format = depth_format_;
    depth_view_info.subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};
    depth_view_ = device_.device().createImageView(depth_view_info);

    // Sampler to read color image
    vk::SamplerCreateInfo sampler_info{};
    sampler_info.magFilter = vk::Filter::eLinear;
    sampler_info.minFilter = vk::Filter::eLinear;
    sampler_info.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sampler_info.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    sampler_ = device_.device().createSampler(sampler_info);
}

void OffscreenPass::beginRendering(const vk::raii::CommandBuffer& cmd) {
    transitionImage(cmd, *color_image_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal);

    transitionImage(cmd, *depth_image_,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::ImageAspectFlagBits::eDepth);

    vk::RenderingAttachmentInfo color_attachment{};
    color_attachment.imageView = *color_view_;
    color_attachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.clearValue.color = vk::ClearColorValue{std::array{0.02f, 0.02f, 0.02f, 1.0f}};

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

void OffscreenPass::endRendering(const vk::raii::CommandBuffer& cmd) {
    cmd.endRendering();

    // Transition color image to shader-readable for ImGui to sample
    transitionImage(cmd, *color_image_,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);
}

void OffscreenPass::resize(uint32_t width, uint32_t height) {
    if (width == width_ && height == height_) return;
    if (width == 0 || height == 0) return;

    device_.device().waitIdle();

    width_ = width;
    height_ = height;

    // Destroy old resources (RAII handles this when we reassign)
    color_view_ = nullptr;
    color_image_ = nullptr;
    color_memory_ = nullptr;
    depth_view_ = nullptr;
    depth_image_ = nullptr;
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

} // namespace mve
