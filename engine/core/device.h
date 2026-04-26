#ifndef MVE_DEVICE_H
#define MVE_DEVICE_H

#include "window.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <optional>
#include <vector>

namespace mve {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool IsComplete() const {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

class Device {
public:
    Device(Window& window);

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    const vk::raii::Device& GetDevice() const { return device_; }
    const vk::raii::PhysicalDevice& GetPhysicalDevice() const { return physical_device_; }
    const vk::raii::Instance& GetInstance() const { return instance_; }
    const vk::raii::SurfaceKHR& GetSurface() const { return surface_; }
    const vk::raii::Queue& GetGraphicsQueue() const { return graphics_queue_; }
    const vk::raii::Queue& GetPresentQueue() const { return present_queue_; }
    const vk::raii::CommandPool& GetCommandPool() const { return command_pool_; }

    QueueFamilyIndices FindQueueFamilies() const { return FindQueueFamilies(*physical_device_); }
    SwapchainSupportDetails QuerySwapchainSupport() const { return QuerySwapchainSupport(*physical_device_); }

    vk::Format FindSupportedFormat(
        const std::vector<vk::Format>& candidates,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features) const;

    uint32_t FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties) const;
    vk::Format FindDepthFormat() const;

    vk::raii::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::raii::CommandBuffer command_buffer);

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool isDeviceSuitable(const vk::raii::PhysicalDevice& device) const;
    QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device) const;
    SwapchainSupportDetails QuerySwapchainSupport(vk::PhysicalDevice device) const;
    bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device) const;
    bool checkValidationLayerSupport() const;

    Window& window_;

    vk::raii::Context context_;
    vk::raii::Instance instance_{nullptr};
    vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
    vk::raii::SurfaceKHR surface_{nullptr};
    vk::raii::PhysicalDevice physical_device_{nullptr};
    vk::raii::Device device_{nullptr};
    vk::raii::Queue graphics_queue_{nullptr};
    vk::raii::Queue present_queue_{nullptr};
    vk::raii::CommandPool command_pool_{nullptr};

    const std::vector<const char*> validation_layers_ = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
    static constexpr bool enable_validation_ = false;
#else
    static constexpr bool enable_validation_ = true;
#endif
};

} // namespace mve

#endif // MVE_DEVICE_H
