#ifndef MVE_DESCRIPTOR_H
#define MVE_DESCRIPTOR_H

#include "../core/device.h"

namespace mve {

class DescriptorSetLayout {
public:
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
};

class DescriptorPool {
public:
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;
};

} // namespace mve

#endif // MVE_DESCRIPTOR_H
