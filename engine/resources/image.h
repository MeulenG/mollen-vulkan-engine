#ifndef MVE_IMAGE_H
#define MVE_IMAGE_H

#include "../core/device.h"
#include "../formats/blp_loader.h"

#include <vector>

namespace mve {

class Image {
public:
    // Create from raw RGBA pixel data
    Image(Device& device, uint32_t width, uint32_t height, const uint8_t* pixels);

    // Create from a parsed BLP texture (handles both compressed DXT and uncompressed)
    Image(Device& device, const BlpTexture& blp);

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) = default;
    Image& operator=(Image&&) = default;

    const vk::raii::ImageView& imageView() const { return image_view_; }
    const vk::raii::Sampler& sampler() const { return sampler_; }

    vk::DescriptorImageInfo descriptorInfo() const {
        return {*sampler_, *image_view_, vk::ImageLayout::eShaderReadOnlyOptimal};
    }

    // Generate a simple checkerboard test texture
    static Image createCheckerboard(Device& device, uint32_t size = 64, uint32_t cell_size = 8);

private:
    void createImage(uint32_t width, uint32_t height, vk::Format format, uint32_t mip_levels = 1);
    void createImageView(vk::Format format, uint32_t mip_levels = 1);
    void createSampler(uint32_t mip_levels = 1);
    void uploadPixels(const uint8_t* pixels, uint32_t width, uint32_t height);
    void uploadCompressed(const BlpTexture& blp);
    void transitionLayout(vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t mip_levels = 1);

    Device& device_;
    vk::raii::Image image_{nullptr};
    vk::raii::DeviceMemory memory_{nullptr};
    vk::raii::ImageView image_view_{nullptr};
    vk::raii::Sampler sampler_{nullptr};
};

} // namespace mve

#endif // MVE_IMAGE_H
