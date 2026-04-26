#include "renderer.h"

#include <stdexcept>

namespace mve {

Renderer::Renderer(Window& window, Device& device)
    : window_{window}, device_{device} {
    depth_format_ = device_.FindDepthFormat();
    swapchain_ = std::make_unique<Swapchain>(device_, window_.GetExtent());
    createCommandBuffers();
    createDepthResources();
}

void Renderer::createCommandBuffers() {
    vk::CommandBufferAllocateInfo alloc_info{
        *device_.GetCommandPool(),
        vk::CommandBufferLevel::ePrimary,
        swapchain_->ImageCount()
    };

    command_buffers_ = device_.GetDevice().allocateCommandBuffers(alloc_info);
}

void Renderer::createDepthResources() {
    auto extent = swapchain_->extent();

    vk::ImageCreateInfo image_info{};
    image_info.imageType = vk::ImageType::e2D;
    image_info.format = depth_format_;
    image_info.extent = vk::Extent3D{extent.width, extent.height, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.tiling = vk::ImageTiling::eOptimal;
    image_info.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

    depth_image_ = device_.GetDevice().createImage(image_info);

    auto mem_reqs = depth_image_.getMemoryRequirements();
    depth_memory_ = device_.GetDevice().allocateMemory({
        mem_reqs.size,
        device_.FindMemoryType(mem_reqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    });
    depth_image_.bindMemory(*depth_memory_, 0);

    vk::ImageViewCreateInfo view_info{};
    view_info.image = *depth_image_;
    view_info.viewType = vk::ImageViewType::e2D;
    view_info.format = depth_format_;
    view_info.subresourceRange = {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1};

    depth_image_view_ = device_.GetDevice().createImageView(view_info);
}

void Renderer::recreateSwapchain() {
    auto extent = window_.GetExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window_.GetExtent();
        glfwWaitEvents();
    }

    device_.GetDevice().waitIdle();

    command_buffers_.clear();
    depth_image_view_ = nullptr;
    depth_image_ = nullptr;
    depth_memory_ = nullptr;

    swapchain_ = std::make_unique<Swapchain>(device_, extent);
    createCommandBuffers();
    createDepthResources();
}

bool Renderer::BeginFrame(vk::raii::CommandBuffer** out_command_buffer) {
    auto result = swapchain_->AcquireNextImage(&current_image_index_);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapchain();
        return false;
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    is_frame_started_ = true;

    auto& command_buffer = command_buffers_[swapchain_->CurrentFrame()];
    command_buffer.reset();
    command_buffer.begin({});

    *out_command_buffer = &command_buffer;
    return true;
}

void Renderer::BeginRendering(const vk::raii::CommandBuffer& command_buffer, bool with_depth) {
    vk::Image image = swapchain_->GetImage(current_image_index_);

    transitionImage(command_buffer, image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal);

    vk::RenderingAttachmentInfo depth_attachment{};
    if (with_depth) {
        transitionImage(command_buffer, *depth_image_,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthAttachmentOptimal,
            vk::ImageAspectFlagBits::eDepth);

        depth_attachment.imageView = *depth_image_view_;
        depth_attachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};
    }

    vk::RenderingAttachmentInfo color_attachment{};
    color_attachment.imageView = *swapchain_->GetImageView(current_image_index_);
    color_attachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.clearValue.color = vk::ClearColorValue{std::array{0.01f, 0.01f, 0.01f, 1.0f}};

    vk::RenderingInfo rendering_info{};
    rendering_info.renderArea = vk::Rect2D{vk::Offset2D{0, 0}, swapchain_->extent()};
    rendering_info.layerCount = 1;
    rendering_info.setColorAttachments(color_attachment);
    if (with_depth) {
        rendering_info.pDepthAttachment = &depth_attachment;
    }

    command_buffer.beginRendering(rendering_info);

    vk::Viewport viewport{
        0.0f, 0.0f,
        static_cast<float>(swapchain_->extent().width),
        static_cast<float>(swapchain_->extent().height),
        0.0f, 1.0f
    };
    command_buffer.setViewport(0, viewport);

    vk::Rect2D scissor{vk::Offset2D{0, 0}, swapchain_->extent()};
    command_buffer.setScissor(0, scissor);
}

void Renderer::EndRendering(const vk::raii::CommandBuffer& command_buffer) {
    command_buffer.endRendering();

    transitionImage(command_buffer,
        swapchain_->GetImage(current_image_index_),
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR);
}

void Renderer::EndFrame(const vk::raii::CommandBuffer& command_buffer) {
    command_buffer.end();

    auto result = swapchain_->SubmitCommandBuffer(command_buffer, current_image_index_);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window_.WasResized()) {
        window_.ResetResizedFlag();
        recreateSwapchain();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image");
    }

    is_frame_started_ = false;
}

void Renderer::transitionImage(
    const vk::raii::CommandBuffer& cmd,
    vk::Image image,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
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
        barrier.srcAccessMask = {};
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    } else if (old_layout == vk::ImageLayout::eColorAttachmentOptimal &&
               new_layout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
        barrier.dstAccessMask = {};
    } else if (old_layout == vk::ImageLayout::eUndefined &&
               new_layout == vk::ImageLayout::eDepthAttachmentOptimal) {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.srcAccessMask = {};
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
        barrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
    }

    vk::DependencyInfo dep_info{};
    dep_info.setImageMemoryBarriers(barrier);

    cmd.pipelineBarrier2(dep_info);
}

} // namespace mve
