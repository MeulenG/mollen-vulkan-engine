#include "imgui_context.h"
#include "editor_style.h"

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
    // ImGui's descriptor pool - covers all binding types ImGui uses.
    // eFreeDescriptorSet is required because ImGui frees descriptor sets
    // when textures are unregistered (which happens every time the
    // editor's viewport panel is resized).
    //
    // 1024 sets gives ~10x the per-frame churn budget; 100 was too tight
    // and exhausted on a fast viewport-panel drag. When the pool runs
    // out, vkAllocateDescriptorSets returns OUT_OF_POOL_MEMORY but
    // ImGui's AddTexture call site doesn't check the result, returning
    // an uninitialized handle - leading to follow-on
    // 'Invalid VkDescriptorPool Object 0x33...0033' validation errors.
    constexpr uint32_t kImGuiPoolSize = 1024;
    std::vector<vk::DescriptorPoolSize> pool_sizes = {
        {vk::DescriptorType::eSampler,                kImGuiPoolSize},
        {vk::DescriptorType::eCombinedImageSampler,   kImGuiPoolSize},
        {vk::DescriptorType::eSampledImage,           kImGuiPoolSize},
        {vk::DescriptorType::eStorageImage,           kImGuiPoolSize},
        {vk::DescriptorType::eUniformTexelBuffer,     kImGuiPoolSize},
        {vk::DescriptorType::eStorageTexelBuffer,     kImGuiPoolSize},
        {vk::DescriptorType::eUniformBuffer,          kImGuiPoolSize},
        {vk::DescriptorType::eStorageBuffer,          kImGuiPoolSize},
        {vk::DescriptorType::eUniformBufferDynamic,   kImGuiPoolSize},
        {vk::DescriptorType::eStorageBufferDynamic,   kImGuiPoolSize},
        {vk::DescriptorType::eInputAttachment,        kImGuiPoolSize},
    };

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = kImGuiPoolSize;
    pool_info.setPoolSizes(pool_sizes);

    descriptor_pool_ = device.GetDevice().createDescriptorPool(pool_info);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Apply editor theme + load Inter / Fork Awesome merged atlas. Both
    // must run before ImGui_ImplVulkan_Init, which uploads the atlas to
    // GPU memory and bakes the font texture.
    editor_style::Apply(*ImGui::GetCurrentContext());
    editor_style::LoadFonts(*io.Fonts);

    InitBackends(window, device, swapchain_format);
}

std::string ImGuiContext::SaveSettings() const {
    size_t len = 0;
    const char* data = ImGui::SaveIniSettingsToMemory(&len);
    return std::string(data, len);
}

void ImGuiContext::ShutdownForReload() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiContext::ReinitForReload(Window& window, Device& device,
                                   vk::Format swapchain_format,
                                   const std::string& ini) {
    // Fresh context in THIS module: settings handlers, allocator and
    // backend data all resolve to live code. Layout state comes back in
    // via the ini text (must load after CreateContext, before NewFrame -
    // it also marks settings as loaded so the disk ini isn't re-read).
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    editor_style::Apply(*ImGui::GetCurrentContext());
    editor_style::LoadFonts(*io.Fonts);

    ImGui::LoadIniSettingsFromMemory(ini.c_str(), ini.size());

    InitBackends(window, device, swapchain_format);
}

void ImGuiContext::InitBackends(Window& window, Device& device, vk::Format swapchain_format) {
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

    return (ImTextureID)ds;
}

void ImGuiContext::UnregisterTexture(ImTextureID tex) {
    if (tex == ImTextureID_Invalid) return;
    ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(tex));
}

} // namespace mve
