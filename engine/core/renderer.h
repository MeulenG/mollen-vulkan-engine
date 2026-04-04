#ifndef MVE_RENDERER_H
#define MVE_RENDERER_H

#include "device.h"
#include "swapchain.h"
#include "window.h"

#include <memory>
#include <vector>

namespace mve {

class Renderer {
public:
    Renderer(Window& window, Device& device);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool beginFrame(vk::raii::CommandBuffer** out_command_buffer);
    void beginRendering(const vk::raii::CommandBuffer& command_buffer);
    void endRendering(const vk::raii::CommandBuffer& command_buffer);
    void endFrame(const vk::raii::CommandBuffer& command_buffer);

    vk::Extent2D getSwapchainExtent() const { return swapchain_->extent(); }
    vk::Format getSwapchainImageFormat() const { return swapchain_->imageFormat(); }
    vk::Format getDepthFormat() const { return depth_format_; }

private:
    void createCommandBuffers();
    void createDepthResources();
    void recreateSwapchain();

    void transitionImage(
        const vk::raii::CommandBuffer& cmd,
        vk::Image image,
        vk::ImageLayout old_layout,
        vk::ImageLayout new_layout,
        vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

    Window& window_;
    Device& device_;
    std::unique_ptr<Swapchain> swapchain_;

    std::vector<vk::raii::CommandBuffer> command_buffers_;
    uint32_t current_image_index_ = 0;
    bool is_frame_started_ = false;

    vk::Format depth_format_;
    vk::raii::Image depth_image_{nullptr};
    vk::raii::DeviceMemory depth_memory_{nullptr};
    vk::raii::ImageView depth_image_view_{nullptr};
};

} // namespace mve

#endif // MVE_RENDERER_H
