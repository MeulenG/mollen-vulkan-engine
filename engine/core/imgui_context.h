#ifndef MVE_IMGUI_CONTEXT_H
#define MVE_IMGUI_CONTEXT_H

#include "device.h"
#include "window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace mve {

// Manages ImGui lifecycle: init, new frame, render, shutdown.
class ImGuiContext {
public:
    ImGuiContext(Window& window, Device& device, vk::Format swapchain_format);
    ~ImGuiContext();

    ImGuiContext(const ImGuiContext&) = delete;
    ImGuiContext& operator=(const ImGuiContext&) = delete;

    void NewFrame();

    void Render(const vk::raii::CommandBuffer& cmd);

    ImTextureID RegisterTexture(vk::Sampler sampler, vk::ImageView view,
                                vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

private:
    void initImGui(Window& window, Device& device, vk::Format swapchain_format);

    vk::raii::DescriptorPool descriptor_pool_{nullptr};
};

} // namespace mve

#endif // MVE_IMGUI_CONTEXT_H
