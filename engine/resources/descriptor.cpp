#include "descriptor.h"

#include <stdexcept>

namespace mve {

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::addBinding(
    uint32_t binding,
    vk::DescriptorType type,
    vk::ShaderStageFlags stage_flags,
    uint32_t count) {

    vk::DescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = binding;
    layout_binding.descriptorType = type;
    layout_binding.descriptorCount = count;
    layout_binding.stageFlags = stage_flags;

    bindings_.push_back(layout_binding);
    return *this;
}

vk::raii::DescriptorSetLayout DescriptorSetLayoutBuilder::build() {
    vk::DescriptorSetLayoutCreateInfo create_info{};
    create_info.setBindings(bindings_);

    return device_.device().createDescriptorSetLayout(create_info);
}

DescriptorPool::DescriptorPool(
    Device& device,
    uint32_t max_sets,
    const std::vector<vk::DescriptorPoolSize>& pool_sizes)
    : device_{device} {

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.setPoolSizes(pool_sizes);
    pool_info.maxSets = max_sets;

    pool_ = device_.device().createDescriptorPool(pool_info);
}

vk::raii::DescriptorSet DescriptorPool::allocateSet(const vk::raii::DescriptorSetLayout& layout) {
    vk::DescriptorSetLayout raw_layout = *layout;

    vk::DescriptorSetAllocateInfo alloc_info{};
    alloc_info.descriptorPool = *pool_;
    alloc_info.descriptorSetCount = 1;
    alloc_info.setSetLayouts(raw_layout);

    auto sets = device_.device().allocateDescriptorSets(alloc_info);
    return std::move(sets[0]);
}

DescriptorWriter& DescriptorWriter::writeBuffer(
    uint32_t binding,
    const vk::DescriptorBufferInfo& buffer_info,
    vk::DescriptorType type) {

    buffer_infos_.push_back(buffer_info);

    vk::WriteDescriptorSet write{};
    write.dstBinding = binding;
    write.descriptorCount = 1;
    write.descriptorType = type;
    // pBufferInfo will be set in apply() using the stored index
    writes_.push_back(write);

    return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(
    uint32_t binding,
    const vk::DescriptorImageInfo& image_info,
    vk::DescriptorType type) {

    image_infos_.push_back(image_info);

    vk::WriteDescriptorSet write{};
    write.dstBinding = binding;
    write.descriptorCount = 1;
    write.descriptorType = type;
    writes_.push_back(write);

    return *this;
}

void DescriptorWriter::apply(const vk::raii::Device& device, const vk::raii::DescriptorSet& set) {
    uint32_t buf_idx = 0;
    uint32_t img_idx = 0;

    for (auto& write : writes_) {
        write.dstSet = *set;

        if (write.descriptorType == vk::DescriptorType::eUniformBuffer ||
            write.descriptorType == vk::DescriptorType::eStorageBuffer ||
            write.descriptorType == vk::DescriptorType::eUniformBufferDynamic) {
            write.pBufferInfo = &buffer_infos_[buf_idx++];
        } else {
            write.pImageInfo = &image_infos_[img_idx++];
        }
    }

    device.updateDescriptorSets(writes_, nullptr);
}

} // namespace mve
