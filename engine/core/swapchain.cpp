#include "swapchain.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace mve {

Swapchain::Swapchain(Device& device, VkExtent2D window_extent)
    : device_{device}, window_extent_{window_extent} {
    createSwapchain();
    createImageViews();
    createSyncObjects();
}

Swapchain::~Swapchain() {
    for (auto view : image_views_) {
        vkDestroyImageView(device_.device(), view, nullptr);
    }

    vkDestroySwapchainKHR(device_.device(), swapchain_, nullptr);

    for (size_t i = 0; i < image_available_semaphores_.size(); i++) {
        vkDestroySemaphore(device_.device(), image_available_semaphores_[i], nullptr);
        vkDestroySemaphore(device_.device(), render_finished_semaphores_[i], nullptr);
        vkDestroyFence(device_.device(), in_flight_fences_[i], nullptr);
    }
}

void Swapchain::createSwapchain() {
    SwapchainSupportDetails support = device_.querySwapchainSupport();

    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(support.formats);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(support.present_modes);
    VkExtent2D extent = chooseSwapExtent(support.capabilities);

    uint32_t image_count = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
        image_count = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = device_.surface();
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = device_.findQueueFamilies();
    uint32_t family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_.device(), &create_info, nullptr, &swapchain_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(device_.device(), swapchain_, &image_count, nullptr);
    images_.resize(image_count);
    vkGetSwapchainImagesKHR(device_.device(), swapchain_, &image_count, images_.data());

    image_format_ = surface_format.format;
    extent_ = extent;
}

void Swapchain::createImageViews() {
    image_views_.resize(images_.size());

    for (size_t i = 0; i < images_.size(); i++) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = images_[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = image_format_;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_.device(), &view_info, nullptr, &image_views_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }
}

void Swapchain::createSyncObjects() {
    // All sync objects sized to swapchain image count to avoid semaphore reuse conflicts
    uint32_t count = static_cast<uint32_t>(images_.size());
    image_available_semaphores_.resize(count);
    render_finished_semaphores_.resize(count);
    in_flight_fences_.resize(count);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < count; i++) {
        if (vkCreateSemaphore(device_.device(), &semaphore_info, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_.device(), &semaphore_info, nullptr, &render_finished_semaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(device_.device(), &fence_info, nullptr, &in_flight_fences_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects");
        }
    }
}

VkResult Swapchain::acquireNextImage(uint32_t* image_index) {
    vkWaitForFences(device_.device(), 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

    // Use per-image semaphore indexed by current_frame_ for the acquire.
    // After acquire succeeds, we store the image index so submit knows which semaphore to wait on.
    VkResult result = vkAcquireNextImageKHR(
        device_.device(),
        swapchain_,
        UINT64_MAX,
        image_available_semaphores_[current_frame_],
        VK_NULL_HANDLE,
        image_index);

    current_image_index_ = *image_index;
    return result;
}

VkResult Swapchain::submitCommandBuffer(VkCommandBuffer buffer, uint32_t image_index) {
    vkResetFences(device_.device(), 1, &in_flight_fences_[current_frame_]);

    VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {render_finished_semaphores_[current_frame_]};

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(device_.graphicsQueue(), 1, &submit_info, in_flight_fences_[current_frame_]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkSwapchainKHR swapchains[] = {swapchain_};

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;

    VkResult result = vkQueuePresentKHR(device_.presentQueue(), &present_info);

    current_frame_ = (current_frame_ + 1) % static_cast<uint32_t>(images_.size());

    return result;
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) {
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual = window_extent_;
    actual.width = std::clamp(actual.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual.height = std::clamp(actual.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual;
}

} // namespace mve
