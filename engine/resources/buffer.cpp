#include "buffer.h"

#include <cstring>
#include <stdexcept>

namespace mve {

Buffer::Buffer(
    Device& device,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties)
    : device_{device}, size_{size} {

    buffer_ = device_.device().createBuffer({{}, size, usage});

    auto mem_requirements = buffer_.getMemoryRequirements();

    memory_ = device_.device().allocateMemory({
        mem_requirements.size,
        device_.FindMemoryType(mem_requirements.memoryTypeBits, properties)
    });

    buffer_.bindMemory(*memory_, 0);
}

void* Buffer::map() {
    if (!mapped_) {
        mapped_ = memory_.mapMemory(0, size_);
    }
    return mapped_;
}

void Buffer::unmap() {
    if (mapped_) {
        memory_.unmapMemory();
        mapped_ = nullptr;
    }
}

void Buffer::write(const void* data, vk::DeviceSize size) {
    void* dest = map();
    std::memcpy(dest, data, static_cast<size_t>(size));
    unmap();
}

Buffer Buffer::CreateWithStaging(
    Device& device,
    const void* data,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage) {

    Buffer staging{
        device, size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };
    staging.write(data, size);

    Buffer gpu_buffer{
        device, size,
        usage | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    };

    auto cmd = device.BeginSingleTimeCommands();
    cmd.copyBuffer(*staging.buffer(), *gpu_buffer.buffer(), vk::BufferCopy{0, 0, size});
    device.EndSingleTimeCommands(std::move(cmd));

    return gpu_buffer;
}

} // namespace mve
