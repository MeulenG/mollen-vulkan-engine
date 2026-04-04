#ifndef MVE_SWAPCHAIN_H
#define MVE_SWAPCHAIN_H

#include "device.h"

#include <vector>

namespace mve {

class Swapchain {
public:
    Swapchain(Device& device, vk::Extent2D window_extent);

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    vk::Format imageFormat() const { return image_format_; }
    vk::Extent2D extent() const { return extent_; }
    uint32_t imageCount() const { return static_cast<uint32_t>(images_.size()); }

    vk::Image getImage(uint32_t index) const { return images_[index]; }
    const vk::raii::ImageView& getImageView(uint32_t index) const { return image_views_[index]; }

    vk::Result acquireNextImage(uint32_t* image_index);
    vk::Result submitCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t image_index);

    uint32_t currentFrame() const { return current_frame_; }

private:
    void createSwapchain();
    void createImageViews();
    void createSyncObjects();

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& modes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    Device& device_;
    vk::Extent2D window_extent_;

    vk::raii::SwapchainKHR swapchain_{nullptr};
    vk::Format image_format_;
    vk::Extent2D extent_;

    std::vector<vk::Image> images_;
    std::vector<vk::raii::ImageView> image_views_;

    std::vector<vk::raii::Semaphore> image_available_semaphores_;
    std::vector<vk::raii::Semaphore> render_finished_semaphores_;
    std::vector<vk::raii::Fence> in_flight_fences_;

    uint32_t current_frame_ = 0;
};

} // namespace mve

#endif // MVE_SWAPCHAIN_H
