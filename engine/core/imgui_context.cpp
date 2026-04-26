#include "imgui_context.h"

#include <stdexcept>

namespace mve {

ImGuiContext::ImGuiContext(Window& window, Device& device, vk::Format swapchain_format) {
    initImGui(window, device, swapchain_format);
}

ImGuiContext::~ImGuiContext() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiContext::initImGui(Window& window, Device& device, vk::Format swapchain_format) {
    // ImGui needs its own descriptor pool
    std::vector<vk::DescriptorPoolSize> pool_sizes = {
        {vk::DescriptorType::eCombinedImageSampler, 100},
    };

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = 100;
    pool_info.setPoolSizes(pool_sizes);

    descriptor_pool_ = device.GetDevice().createDescriptorPool(pool_info);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;

    ImGui_ImplGlfw_InitForVulkan(window.GetGLFWWindow(), true);

    // No Render Pass
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = *device.GetInstance();
    init_info.PhysicalDevice = *device.GetPhysicalDevice();
    init_info.Device = *device.GetDevice();

    auto indices = device.FindQueueFamilies();
    init_info.QueueFamily = indices.graphics_family.value();
    init_info.Queue = *device.GetGraphicsQueue();
    init_info.DescriptorPool = *descriptor_pool_;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;

    // Set up PipelineInfoMain for dynamic rendering
    VkFormat vk_format = static_cast<VkFormat>(swapchain_format);
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vk_format;

    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiContext::NewFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiContext::Render(const vk::raii::CommandBuffer& cmd) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
}

ImTextureID ImGuiContext::RegisterTexture(vk::Sampler sampler, vk::ImageView view,
                                           vk::ImageLayout layout) {
    VkDescriptorSet ds = ImGui_ImplVulkan_AddTexture(
        static_cast<VkSampler>(sampler),
        static_cast<VkImageView>(view),
        static_cast<VkImageLayout>(layout));

    return static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(ds));
}

} // namespace mve
