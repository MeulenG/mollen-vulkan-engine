#include "swapchain.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace mve {

Swapchain::Swapchain(Device& device, vk::Extent2D window_extent)
    : device_{device}, window_extent_{window_extent} {
    createSwapchain();
    createImageViews();
    createSyncObjects();
}

void Swapchain::createSwapchain() {
    auto support = device_.QuerySwapchainSupport();

    auto surface_format = chooseSwapSurfaceFormat(support.formats);
    auto present_mode = chooseSwapPresentMode(support.present_modes);
    extent_ = chooseSwapExtent(support.capabilities);
    image_format_ = surface_format.format;

    uint32_t image_count = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
        image_count = support.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR create_info{
        {},
        *device_.GetSurface(),
        image_count,
        image_format_,
        surface_format.colorSpace,
        extent_,
        1,
        vk::ImageUsageFlagBits::eColorAttachment
    };

    auto indices = device_.FindQueueFamilies();
    uint32_t family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = family_indices;
    } else {
        create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = present_mode;
    create_info.clipped = vk::True;

    swapchain_ = device_.GetDevice().createSwapchainKHR(create_info);
    images_ = swapchain_.getImages();
}

void Swapchain::createImageViews() {
    image_views_.reserve(images_.size());

    for (auto image : images_) {
        vk::ImageViewCreateInfo view_info{
            {},
            image,
            vk::ImageViewType::e2D,
            image_format_,
            {},
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        };

        image_views_.push_back(device_.GetDevice().createImageView(view_info));
    }
}

void Swapchain::createSyncObjects() {
    uint32_t count = static_cast<uint32_t>(images_.size());

    for (uint32_t i = 0; i < count; i++) {
        image_available_semaphores_.push_back(device_.GetDevice().createSemaphore({}));
        render_finished_semaphores_.push_back(device_.GetDevice().createSemaphore({}));
        in_flight_fences_.push_back(
            device_.GetDevice().createFence({vk::FenceCreateFlagBits::eSignaled}));
    }
}

vk::Result Swapchain::AcquireNextImage(uint32_t* image_index) {
    auto wait_result = device_.GetDevice().waitForFences(
        *in_flight_fences_[current_frame_], vk::True, UINT64_MAX);
    (void)wait_result;

    auto [result, index] = swapchain_.acquireNextImage(
        UINT64_MAX, *image_available_semaphores_[current_frame_]);

    *image_index = index;
    return result;
}

vk::Result Swapchain::SubmitCommandBuffer(const vk::raii::CommandBuffer& buffer, uint32_t image_index) {
    device_.GetDevice().resetFences(*in_flight_fences_[current_frame_]);

    vk::Semaphore wait_semaphores[] = {*image_available_semaphores_[current_frame_]};
    vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signal_semaphores[] = {*render_finished_semaphores_[current_frame_]};
    vk::CommandBuffer cmd = *buffer;

    vk::SubmitInfo submit_info{};
    submit_info.setWaitSemaphores(wait_semaphores);
    submit_info.setWaitDstStageMask(wait_stages);
    submit_info.setCommandBuffers(cmd);
    submit_info.setSignalSemaphores(signal_semaphores);

    device_.GetGraphicsQueue().submit(submit_info, *in_flight_fences_[current_frame_]);

    vk::PresentInfoKHR present_info{};
    present_info.setWaitSemaphores(signal_semaphores);
    vk::SwapchainKHR swapchains[] = {*swapchain_};
    present_info.setSwapchains(swapchains);
    present_info.setImageIndices(image_index);

    vk::Result result;
    try {
        result = device_.GetPresentQueue().presentKHR(present_info);
    } catch (const vk::OutOfDateKHRError&) {
        result = vk::Result::eErrorOutOfDateKHR;
    }

    current_frame_ = (current_frame_ + 1) % static_cast<uint32_t>(images_.size());

    return result;
}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return formats[0];
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& modes) {
    for (const auto& mode : modes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }

    vk::Extent2D actual = window_extent_;
    actual.width = std::clamp(actual.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual.height = std::clamp(actual.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual;
}

} // namespace mve
