#include "device.h"

#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>

namespace mve {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* /*user_data*/) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Vulkan: " << callback_data->pMessage << "\n";
    }
    return VK_FALSE;
}

Device::Device(Window& window) : window_{window} {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

void Device::createInstance() {
    if (enable_validation_ && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested but not available");
    }

    vk::ApplicationInfo app_info{
        "Mollen Wow Tools",
        VK_MAKE_VERSION(0, 1, 0),
        "MollenEngine",
        VK_MAKE_VERSION(0, 1, 0),
        VK_API_VERSION_1_4
    };

    auto extensions = Window::GetRequiredInstanceExtensions();
    if (enable_validation_) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    vk::InstanceCreateInfo create_info{};
    create_info.pApplicationInfo = &app_info;
    if (enable_validation_) {
        create_info.setPEnabledLayerNames(validation_layers_);
    }
    create_info.setPEnabledExtensionNames(extensions);

    // Chain debug messenger for instance creation/destruction
    vk::DebugUtilsMessengerCreateInfoEXT debug_info{
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback
    };

    if (enable_validation_) {
        create_info.pNext = &debug_info;
    }

    instance_ = vk::raii::Instance{context_, create_info};
}

void Device::setupDebugMessenger() {
    if (!enable_validation_)  {
        return;
    }

    vk::DebugUtilsMessengerCreateInfoEXT create_info{
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback
    };

    debug_messenger_ = instance_.createDebugUtilsMessengerEXT(create_info);
}

void Device::RecreateDebugMessenger() {
    if (!enable_validation_) return;
    debug_messenger_ = nullptr;
    setupDebugMessenger();
}

void Device::createSurface() {
    auto raw_surface = window_.createSurface(*instance_);
    surface_ = vk::raii::SurfaceKHR{instance_, raw_surface};
}

void Device::pickPhysicalDevice() {
    auto devices = instance_.enumeratePhysicalDevices();

    if (devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physical_device_ = std::move(device);
            break;
        }
    }

    if (!*physical_device_) {
        throw std::runtime_error("Failed to find a suitable GPU");
    }

    auto props = physical_device_.getProperties();
    std::cout << "Selected GPU: " << props.deviceName << "\n";
}

void Device::createLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(*physical_device_);

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_families = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    float queue_priority = 1.0f;
    for (uint32_t family : unique_families) {
        queue_create_infos.push_back({{}, family, 1, &queue_priority});
    }

    // Enable dynamic rendering (core in Vulkan 1.3+)
    vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features{vk::True};

    // Enable synchronization2 (core in Vulkan 1.3+)
    vk::PhysicalDeviceSynchronization2Features sync2_features{vk::True};
    sync2_features.pNext = &dynamic_rendering_features;

    vk::PhysicalDeviceFeatures device_features{};
    // Anisotropic texture filtering. Every Vulkan 1.0+ desktop GPU
    // supports this; explicit opt-in is required to actually use it.
    // The sampler's maxAnisotropy is then clamped against
    // physicalDeviceLimits.maxSamplerAnisotropy.
    device_features.samplerAnisotropy = vk::True;

    vk::DeviceCreateInfo create_info{};
    create_info.setQueueCreateInfos(queue_create_infos);
    if (enable_validation_) {
        create_info.setPEnabledLayerNames(validation_layers_);
    }
    create_info.setPEnabledExtensionNames(device_extensions_);
    create_info.pEnabledFeatures = &device_features;
    create_info.pNext = &sync2_features;

    device_ = physical_device_.createDevice(create_info);

    graphics_queue_ = device_.getQueue(indices.graphics_family.value(), 0);
    present_queue_ = device_.getQueue(indices.present_family.value(), 0);
}

void Device::createCommandPool() {
    QueueFamilyIndices indices = FindQueueFamilies(*physical_device_);

    vk::CommandPoolCreateInfo pool_info{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        indices.graphics_family.value()
    };

    command_pool_ = device_.createCommandPool(pool_info);
}

bool Device::isDeviceSuitable(const vk::raii::PhysicalDevice& device) const {
    QueueFamilyIndices indices = FindQueueFamilies(*device);

    bool extensions_supported = checkDeviceExtensionSupport(device);

    bool swapchain_adequate = false;
    if (extensions_supported) {
        auto support = QuerySwapchainSupport(*device);
        swapchain_adequate = !support.formats.empty() && !support.present_modes.empty();
    }

    return indices.IsComplete() && extensions_supported && swapchain_adequate;
}

QueueFamilyIndices Device::FindQueueFamilies(vk::PhysicalDevice device) const {
    QueueFamilyIndices indices;

    auto families = device.getQueueFamilyProperties();

    for (uint32_t i = 0; i < families.size(); i++) {
        if (families[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        if (device.getSurfaceSupportKHR(i, *surface_)) {
            indices.present_family = i;
        }

        if (indices.IsComplete()) break;
    }

    return indices;
}

SwapchainSupportDetails Device::QuerySwapchainSupport(vk::PhysicalDevice device) const {
    return {
        device.getSurfaceCapabilitiesKHR(*surface_),
        device.getSurfaceFormatsKHR(*surface_),
        device.getSurfacePresentModesKHR(*surface_)
    };
}

bool Device::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device) const {
    auto available = device.enumerateDeviceExtensionProperties();

    std::set<std::string> required(device_extensions_.begin(), device_extensions_.end());
    for (const auto& ext : available) {
        required.erase(ext.extensionName);
    }

    return required.empty();
}

bool Device::checkValidationLayerSupport() const {
    auto available = context_.enumerateInstanceLayerProperties();

    for (const char* name : validation_layers_) {
        bool found = false;
        for (const auto& layer : available) {
            if (strcmp(name, layer.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return true;
}

vk::Format Device::FindSupportedFormat(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features) const {
    for (vk::Format format : candidates) {
        auto props = physical_device_.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Failed to find supported format");
}

vk::Format Device::FindDepthFormat() const {
    return FindSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

uint32_t Device::FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties) const {
    auto mem_props = physical_device_.getMemoryProperties();

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

vk::raii::CommandBuffer Device::BeginSingleTimeCommands() {
    vk::CommandBufferAllocateInfo alloc_info{*command_pool_, vk::CommandBufferLevel::ePrimary, 1};

    auto buffers = device_.allocateCommandBuffers(alloc_info);
    auto command_buffer = std::move(buffers[0]);

    command_buffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    return command_buffer;
}

void Device::EndSingleTimeCommands(vk::raii::CommandBuffer command_buffer) {
    command_buffer.end();

    vk::SubmitInfo submit_info{};
    submit_info.setCommandBuffers(*command_buffer);

    graphics_queue_.submit(submit_info);
    graphics_queue_.waitIdle();
}

} // namespace mve
