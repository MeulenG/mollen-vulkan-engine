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

    // Free a descriptor set previously returned by RegisterTexture.
    // Safe to call with ImTextureID_Invalid (no-op). The caller MUST ensure
    // the GPU is idle (e.g. via OffscreenPass::Resize -> waitIdle) before
    // calling - otherwise the descriptor set may still be in flight.
    //
    // Failing to unregister leaks one descriptor set per call to register;
    // the pool caps at 100, so a few minutes of dragging a window can
    // exhaust it and produce uninitialized handles.
    void UnregisterTexture(ImTextureID tex);

private:
    void initImGui(Window& window, Device& device, vk::Format swapchain_format);

    vk::raii::DescriptorPool descriptor_pool_{nullptr};
};

} // namespace mve

#endif // MVE_IMGUI_CONTEXT_H
