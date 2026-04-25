#ifndef MVE_OFFSCREEN_PASS_H
#define MVE_OFFSCREEN_PASS_H

#include "device.h"

namespace mve {

// Renders the 3D scene to an offscreen image instead of the swapchain.
// The resulting image is used as a texture in an ImGui panel.
class OffscreenPass {
public:
    OffscreenPass(Device& device, uint32_t width, uint32_t height,
                  vk::Format color_format, vk::Format depth_format);

    OffscreenPass(const OffscreenPass&) = delete;
    OffscreenPass& operator=(const OffscreenPass&) = delete;

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    vk::Format ColorFormat() const { return color_format_; }
    vk::Format DepthFormat() const { return depth_format_; }

    const vk::raii::ImageView& ColorImageView() const { return color_view_; }
    const vk::raii::Sampler& sampler() const { return sampler_; }

    // Returns a descriptor suitable for ImGui::Image()
    vk::DescriptorImageInfo DescriptorInfo() const {
        return {*sampler_, *color_view_, vk::ImageLayout::eShaderReadOnlyOptimal};
    }

    // Begin recording: transitions color+depth to attachment layout
    void BeginRendering(const vk::raii::CommandBuffer& cmd);

    // End recording: transitions color to shader-read layout (for ImGui sampling)
    void EndRendering(const vk::raii::CommandBuffer& cmd);

    void resize(uint32_t width, uint32_t height);

private:
    void createResources();
    void transitionImage(const vk::raii::CommandBuffer& cmd, vk::Image image,
                         vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                         vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

    Device& device_;
    uint32_t width_, height_;
    vk::Format color_format_, depth_format_;

    vk::raii::Image color_image_{nullptr};
    vk::raii::DeviceMemory color_memory_{nullptr};
    vk::raii::ImageView color_view_{nullptr};

    vk::raii::Image depth_image_{nullptr};
    vk::raii::DeviceMemory depth_memory_{nullptr};
    vk::raii::ImageView depth_view_{nullptr};

    vk::raii::Sampler sampler_{nullptr};
};

} // namespace mve

#endif // MVE_OFFSCREEN_PASS_H
