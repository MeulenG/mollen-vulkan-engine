#ifndef MVE_BUFFER_H
#define MVE_BUFFER_H

#include "../core/device.h"

namespace mve {

class Buffer {
public:
    Buffer(
        Device& device,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;

    const vk::raii::Buffer& buffer() const { return buffer_; }
    vk::DeviceSize size() const { return size_; }

    void* map();
    void unmap();
    void write(const void* data, vk::DeviceSize size);

    static Buffer CreateWithStaging(
        Device& device,
        const void* data,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage);

private:
    Device& device_;
    vk::DeviceSize size_;
    vk::raii::Buffer buffer_{nullptr};
    vk::raii::DeviceMemory memory_{nullptr};
    void* mapped_ = nullptr;
};

} // namespace mve

#endif // MVE_BUFFER_H
