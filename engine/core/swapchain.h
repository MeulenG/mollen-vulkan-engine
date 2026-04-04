#ifndef MVE_SWAPCHAIN_H
#define MVE_SWAPCHAIN_H

#include "device.h"

#include <vector>

namespace mve {

class Swapchain {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    Swapchain(Device& device, VkExtent2D window_extent);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    VkSwapchainKHR swapchain() const { return swapchain_; }
    VkFormat imageFormat() const { return image_format_; }
    VkExtent2D extent() const { return extent_; }
    uint32_t imageCount() const { return static_cast<uint32_t>(images_.size()); }

    VkImage getImage(uint32_t index) const { return images_[index]; }
    VkImageView getImageView(uint32_t index) const { return image_views_[index]; }

    VkResult acquireNextImage(uint32_t* image_index);
    VkResult submitCommandBuffer(VkCommandBuffer buffer, uint32_t image_index);

    uint32_t currentFrame() const { return current_frame_; }

private:
    void createSwapchain();
    void createImageViews();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    Device& device_;
    VkExtent2D window_extent_;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat image_format_;
    VkExtent2D extent_;

    std::vector<VkImage> images_;
    std::vector<VkImageView> image_views_;

    std::vector<VkSemaphore> image_available_semaphores_;  // per swapchain image
    std::vector<VkSemaphore> render_finished_semaphores_;  // per frame in flight
    std::vector<VkFence> in_flight_fences_;  // per frame in flight

    uint32_t current_frame_ = 0;
    uint32_t current_image_index_ = 0;
};

} // namespace mve

#endif // MVE_SWAPCHAIN_H
