#ifndef MVE_OFFSCREEN_PASS_H
#define MVE_OFFSCREEN_PASS_H

#include "device.h"

#include <string>

namespace mve {

// Renders the 3D scene to an offscreen image instead of the swapchain.
// The resulting image is used as a texture in an ImGui panel.
class OffscreenPass {
public:
    OffscreenPass(Device& device, uint32_t width, uint32_t height,
                  vk::Format color_format, vk::Format depth_format);

    OffscreenPass(const OffscreenPass&) = delete;
    OffscreenPass& operator=(const OffscreenPass&) = delete;

    uint32_t Width() const { return width_; }
    uint32_t Height() const { return height_; }
    vk::Format ColorFormat() const { return color_format_; }
    vk::Format DepthFormat() const { return depth_format_; }

    const vk::raii::ImageView& ColorImageView() const { return color_view_; }
    const vk::raii::Sampler& GetSampler() const { return sampler_; }

    // Returns a descriptor suitable for ImGui::Image()
    vk::DescriptorImageInfo DescriptorInfo() const {
        return {*sampler_, *color_view_, vk::ImageLayout::eShaderReadOnlyOptimal};
    }

    // Begin recording: transitions color+depth to attachment layout
    void BeginRendering(const vk::raii::CommandBuffer& cmd);

    // End recording: transitions color to shader-read layout (for ImGui sampling)
    void EndRendering(const vk::raii::CommandBuffer& cmd);

    void Resize(uint32_t width, uint32_t height);

    // Dump the current contents of the offscreen color image to disk
    // as an 8-bit PNG. Issues a one-shot vkCmdCopyImageToBuffer to a
    // host-visible staging buffer, swaps BGRA -> RGBA, and writes via
    // stb_image_write. Assumes color_image_ is in ShaderReadOnly
    // layout at call time (the post-EndRendering state).
    //
    // Returns true if the file was written successfully.
    //
    // Used by the editor's F12 hotkey (and by autonomous render-quality
    // iteration loops) - this is more reliable than GDI screen capture
    // for Vulkan content, doesn't require the window be in front, and
    // captures exactly what the engine renders.
    bool SaveColorToPng(const std::string& path);

    // 4x MSAA sample count used for the multisample color + depth
    // attachments. Exposed so RenderSystem / PipelineConfig can declare
    // matching rasterization-sample state on every pipeline.
    static constexpr vk::SampleCountFlagBits kSampleCount =
        vk::SampleCountFlagBits::e4;

private:
    void createResources();
    void transitionImage(const vk::raii::CommandBuffer& cmd, vk::Image image,
                         vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                         vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);

    Device& device_;
    uint32_t width_, height_;
    vk::Format color_format_, depth_format_;

    // 4-sample multisample color image. Render targets bind THIS, with
    // color_image_ (single sample, declared below) as the resolve
    // target. We never sample MSAA images directly; the resolve target
    // is what ImGui sees.
    vk::raii::Image msaa_color_image_{nullptr};
    vk::raii::DeviceMemory msaa_color_memory_{nullptr};
    vk::raii::ImageView msaa_color_view_{nullptr};

    // Single-sample resolve target. Pre-2A this was the render target;
    // post-2A it's where the GPU writes resolved MSAA samples. ImGui
    // continues to sample this for the viewport panel.
    vk::raii::Image color_image_{nullptr};
    vk::raii::DeviceMemory color_memory_{nullptr};
    vk::raii::ImageView color_view_{nullptr};

    // 4-sample multisample depth image. No resolve - depth is consumed
    // entirely on-tile (depth test + early-Z) and discarded.
    vk::raii::Image depth_image_{nullptr};
    vk::raii::DeviceMemory depth_memory_{nullptr};
    vk::raii::ImageView depth_view_{nullptr};

    vk::raii::Sampler sampler_{nullptr};
};

} // namespace mve

#endif // MVE_OFFSCREEN_PASS_H
