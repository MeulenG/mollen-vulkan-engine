#ifndef MVE_RENDERER_H
#define MVE_RENDERER_H

#include "device.h"
#include "swapchain.h"
#include "window.h"

#include <functional>
#include <memory>
#include <vector>

namespace mve {

class Renderer {
public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool beginFrame(VkCommandBuffer* out_command_buffer);
    void beginRendering(VkCommandBuffer command_buffer);
    void endRendering(VkCommandBuffer command_buffer);
    void endFrame(VkCommandBuffer command_buffer);

    VkExtent2D getSwapchainExtent() const { return swapchain_->extent(); }
    VkFormat getSwapchainImageFormat() const { return swapchain_->imageFormat(); }

private:
    void createCommandBuffers();
    void recreateSwapchain();

    void transitionImage(
        VkCommandBuffer cmd,
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout);

    Window& window_;
    Device& device_;
    std::unique_ptr<Swapchain> swapchain_;

    std::vector<VkCommandBuffer> command_buffers_;
    uint32_t current_image_index_ = 0;
    bool is_frame_started_ = false;
};

} // namespace mve

#endif // MVE_RENDERER_H
