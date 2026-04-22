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
    glm::vec2 uv;

    // Skinning: which bones affect this vertex, and by how much.
    // Up to 4 bones per vertex (standard for real-time skeletal animation).
    // bone_indices: indices into the bone matrix array
    // bone_weights: how much each bone influences this vertex (sum should = 1.0)
    glm::uvec4 bone_indices{0, 0, 0, 0};
    glm::vec4 bone_weights{1.0f, 0.0f, 0.0f, 0.0f}; // default: 100% bone 0

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

    static Mesh CreatePyramid(Device& device, glm::vec3 color = {0.8f, 0.8f, 0.8f});
    static Mesh createCube(Device& device, glm::vec3 color = {0.8f, 0.8f, 0.8f});
    static Mesh createGroundPlane(Device& device, float size = 20.0f);

private:
    uint32_t index_count_;
    std::unique_ptr<Buffer> vertex_buffer_;
    std::unique_ptr<Buffer> index_buffer_;
};

} // namespace mve

#endif // MVE_MESH_H
