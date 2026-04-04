#include "renderer.h"

#include <stdexcept>

namespace mve {

Renderer::Renderer(Window& window, Device& device)
    : window_{window}, device_{device} {
    swapchain_ = std::make_unique<Swapchain>(device_, window_.getExtent());
    createCommandBuffers();
}

void Renderer::createCommandBuffers() {
    vk::CommandBufferAllocateInfo alloc_info{
        *device_.commandPool(),
        vk::CommandBufferLevel::ePrimary,
        swapchain_->imageCount()
    };

    command_buffers_ = device_.device().allocateCommandBuffers(alloc_info);
}

void Renderer::recreateSwapchain() {
    auto extent = window_.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window_.getExtent();
        glfwWaitEvents();
    }

    device_.device().waitIdle();

    command_buffers_.clear();
    swapchain_ = std::make_unique<Swapchain>(device_, extent);
    createCommandBuffers();
}

bool Renderer::beginFrame(vk::raii::CommandBuffer** out_command_buffer) {
    auto result = swapchain_->acquireNextImage(&current_image_index_);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapchain();
        return false;
    }

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    is_frame_started_ = true;

    auto& command_buffer = command_buffers_[swapchain_->currentFrame()];
    command_buffer.reset();
    command_buffer.begin({});

    *out_command_buffer = &command_buffer;
    return true;
}

void Renderer::beginRendering(const vk::raii::CommandBuffer& command_buffer) {
    vk::Image image = swapchain_->getImage(current_image_index_);

    transitionImage(command_buffer, image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal);

    vk::RenderingAttachmentInfo color_attachment{};
    color_attachment.imageView = *swapchain_->getImageView(current_image_index_);
    color_attachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
    color_attachment.clearValue.color = vk::ClearColorValue{std::array{0.01f, 0.01f, 0.01f, 1.0f}};

    vk::RenderingInfo rendering_info{};
    rendering_info.renderArea = vk::Rect2D{vk::Offset2D{0, 0}, swapchain_->extent()};
    rendering_info.layerCount = 1;
    rendering_info.setColorAttachments(color_attachment);

    command_buffer.beginRendering(rendering_info);

    vk::Viewport viewport{
        0.0f, 0.0f,
        static_cast<float>(swapchain_->extent().width),
        static_cast<float>(swapchain_->extent().height),
        0.0f, 1.0f
    };
    command_buffer.setViewport(0, viewport);

    vk::Rect2D scissor{{0, 0}, swapchain_->extent()};
    command_buffer.setScissor(0, scissor);
}

void Renderer::endRendering(const vk::raii::CommandBuffer& command_buffer) {
    command_buffer.endRendering();

    transitionImage(command_buffer,
        swapchain_->getImage(current_image_index_),
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR);
}

void Renderer::endFrame(const vk::raii::CommandBuffer& command_buffer) {
    command_buffer.end();

    auto result = swapchain_->submitCommandBuffer(command_buffer, current_image_index_);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window_.wasResized()) {
        window_.resetResizedFlag();
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
    vk::ImageLayout new_layout) {

    vk::ImageMemoryBarrier2 barrier{};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

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
    }

    vk::DependencyInfo dep_info{};
    dep_info.setImageMemoryBarriers(barrier);

    cmd.pipelineBarrier2(dep_info);
}

} // namespace mve
