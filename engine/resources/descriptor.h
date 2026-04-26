#ifndef MVE_DESCRIPTOR_H
#define MVE_DESCRIPTOR_H

#include "../core/device.h"

#include <unordered_map>
#include <vector>

namespace mve {

// Builds a descriptor set layout — defines the "interface" of what a shader expects.
// Example: binding 0 = uniform buffer, binding 1 = texture sampler
class DescriptorSetLayoutBuilder {
public:
    DescriptorSetLayoutBuilder(Device& device) : device_{device} {}

    DescriptorSetLayoutBuilder& AddBinding(
        uint32_t binding,
        vk::DescriptorType type,
        vk::ShaderStageFlags stage_flags,
        uint32_t count = 1);

    vk::raii::DescriptorSetLayout Build();

private:
    Device& device_;
    std::vector<vk::DescriptorSetLayoutBinding> bindings_;
};

// Manages a pool of descriptors that sets are allocated from.
class DescriptorPool {
public:
    DescriptorPool(
        Device& device,
        uint32_t max_sets,
        const std::vector<vk::DescriptorPoolSize>& pool_sizes);

    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    vk::raii::DescriptorSet AllocateSet(const vk::raii::DescriptorSetLayout& layout);

private:
    Device& device_;
    vk::raii::DescriptorPool pool_{nullptr};
};

// Helper to write data into a descriptor set.
class DescriptorWriter {
public:
    DescriptorWriter() = default;

    DescriptorWriter& WriteBuffer(
        uint32_t binding,
        const vk::DescriptorBufferInfo& buffer_info,
        vk::DescriptorType type = vk::DescriptorType::eUniformBuffer);

    DescriptorWriter& WriteImage(
        uint32_t binding,
        const vk::DescriptorImageInfo& image_info,
        vk::DescriptorType type = vk::DescriptorType::eCombinedImageSampler);

    void Apply(const vk::raii::Device& device, const vk::raii::DescriptorSet& set);

private:
    std::vector<vk::WriteDescriptorSet> writes_;
    // Keep infos alive until apply() is called
    std::vector<vk::DescriptorBufferInfo> buffer_infos_;
    std::vector<vk::DescriptorImageInfo> image_infos_;
};

} // namespace mve

#endif // MVE_DESCRIPTOR_H
