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

    // Release a previously-registered texture's descriptor set back to
    // ImGui's pool. Mandatory before re-registering when the underlying
    // image view changes (e.g. offscreen viewport resize). Without this,
    // every resize leaks one descriptor set and eventually exhausts the
    // pool, after which vkAllocateDescriptorSets returns
    // OUT_OF_POOL_MEMORY and ImGui hands back an uninitialized handle.
    void UnregisterTexture(ImTextureID id);

private:
    void initImGui(Window& window, Device& device, vk::Format swapchain_format);

    vk::raii::DescriptorPool descriptor_pool_{nullptr};
};

} // namespace mve

#endif // MVE_IMGUI_CONTEXT_H
