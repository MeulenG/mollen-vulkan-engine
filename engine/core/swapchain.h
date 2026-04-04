#ifndef MVE_SWAPCHAIN_H
#define MVE_SWAPCHAIN_H

#include "device.h"

namespace mve {

class Swapchain {
public:
    Swapchain(Device& device, VkExtent2D window_extent);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

private:
    Device& device_;
};

} // namespace mve

#endif // MVE_SWAPCHAIN_H
