#ifndef MVE_MESH_H
#define MVE_MESH_H

#include "../core/device.h"
#include "../resources/buffer.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>

namespace mve {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
};

class Mesh {
public:
    Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    void bind(const vk::raii::CommandBuffer& command_buffer) const;
    void draw(const vk::raii::CommandBuffer& command_buffer) const;

    static Mesh createCube(Device& device, glm::vec3 color = {0.8f, 0.8f, 0.8f});

private:
    uint32_t index_count_;
    std::unique_ptr<Buffer> vertex_buffer_;
    std::unique_ptr<Buffer> index_buffer_;
};

} // namespace mve

#endif // MVE_MESH_H
