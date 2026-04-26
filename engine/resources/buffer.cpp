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

    buffer_ = device_.GetDevice().createBuffer({{}, size, usage});

    auto mem_requirements = buffer_.getMemoryRequirements();

    memory_ = device_.GetDevice().allocateMemory({
        mem_requirements.size,
        device_.FindMemoryType(mem_requirements.memoryTypeBits, properties)
    });

    buffer_.bindMemory(*memory_, 0);
}

void* Buffer::Map() {
    if (!mapped_) {
        mapped_ = memory_.mapMemory(0, size_);
    }
    return mapped_;
}

void Buffer::Unmap() {
    if (mapped_) {
        memory_.unmapMemory();
        mapped_ = nullptr;
    }
}

void Buffer::Write(const void* data, vk::DeviceSize size) {
    void* dest = Map();
    std::memcpy(dest, data, static_cast<size_t>(size));
    Unmap();
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
    staging.Write(data, size);

    Buffer gpu_buffer{
        device, size,
        usage | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    };

    auto cmd = device.BeginSingleTimeCommands();
    cmd.copyBuffer(*staging.GetBuffer(), *gpu_buffer.GetBuffer(), vk::BufferCopy{0, 0, size});
    device.EndSingleTimeCommands(std::move(cmd));

    return gpu_buffer;
}

} // namespace mve
