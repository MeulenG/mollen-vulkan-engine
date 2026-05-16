#include "mesh.h"

namespace mve {

std::vector<vk::VertexInputBindingDescription> Vertex::GetBindingDescriptions() {
    return {{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};
}

std::vector<vk::VertexInputAttributeDescription> Vertex::GetAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
        {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
        {2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)},
        {3, 0, vk::Format::eR32G32Sfloat,    offsetof(Vertex, uv)},
        {4, 0, vk::Format::eR32G32B32A32Uint,  offsetof(Vertex, bone_indices)},
        {5, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, bone_weights)},
    };
}

Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : Mesh(device, vertices.data(), sizeof(Vertex) * vertices.size(), indices) {}

Mesh::Mesh(Device& device, const void* vertex_data, size_t vertex_bytes,
           const std::vector<uint32_t>& indices)
    : index_count_{static_cast<uint32_t>(indices.size())} {

    vertex_buffer_ = std::make_unique<Buffer>(Buffer::CreateWithStaging(
        device,
        vertex_data,
        vertex_bytes,
        vk::BufferUsageFlagBits::eVertexBuffer));

    index_buffer_ = std::make_unique<Buffer>(Buffer::CreateWithStaging(
        device,
        indices.data(),
        sizeof(uint32_t) * indices.size(),
        vk::BufferUsageFlagBits::eIndexBuffer));
}

void Mesh::Bind(const vk::raii::CommandBuffer& command_buffer) const {
    command_buffer.bindVertexBuffers(0, *vertex_buffer_->GetBuffer(), {0});
    command_buffer.bindIndexBuffer(*index_buffer_->GetBuffer(), 0, vk::IndexType::eUint32);
}

void Mesh::Draw(const vk::raii::CommandBuffer& command_buffer) const {
    command_buffer.drawIndexed(index_count_, 1, 0, 0, 0);
}

Mesh Mesh::CreatePyramid(Device& device, glm::vec3 color) {
    std::vector<Vertex> vertices = {
        // Square base
        // Bottom face (y = -0.5)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, color, {0.0f, 1.0f}}, // 0th vertex
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, color, {1.0f, 1.0f}}, // 1st vertex
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, color, {1.0f, 0.0f}}, // 2nd vertex
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, color, {0.0f, 0.0f}}, // 3rd vertex

        // Top Point
        {{0.0f, 1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f}, color, {0.0f, 0.0f}}, // 4th vertex
    };

    std::vector<uint32_t> indices = {
         0, 1, 2, 2, 3, 0, // Base of pyramid
         0, 1, 4,
         0, 3, 4,
         1, 2, 4,
         2, 3, 4,
         
    };

    return Mesh{device, vertices, indices};
}

Mesh Mesh::CreateCube(Device& device, glm::vec3 color) {
    // Each face has its own vertices for correct normals.
    // UV maps the full texture (0,0)-(1,1) onto each face.
    std::vector<Vertex> vertices = {
        // Front face (z = +0.5)
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color, {0.0f, 0.0f}},
        // Back face (z = -0.5)
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color, {0.0f, 0.0f}},
        // Top face (y = +0.5)
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, color, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, color, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, color, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, color, {0.0f, 0.0f}},
        // Bottom face (y = -0.5)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, color, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, color, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, color, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, color, {0.0f, 0.0f}},
        // Right face (x = +0.5)
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, color, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, color, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, color, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, color, {0.0f, 0.0f}},
        // Left face (x = -0.5)
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, color, {0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, color, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, color, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, color, {0.0f, 0.0f}},
    };

    std::vector<uint32_t> indices = {
         0,  1,  2,  2,  3,  0,  // Front
         4,  5,  6,  6,  7,  4,  // Back
         8,  9, 10, 10, 11,  8,  // Top
        12, 13, 14, 14, 15, 12,  // Bottom
        16, 17, 18, 18, 19, 16,  // Right
        20, 21, 22, 22, 23, 20,  // Left
    };

    return Mesh{device, vertices, indices};
}

Mesh Mesh::CreateGroundPlane(Device& device, float size) {
    float h = size * 0.5f;
    glm::vec3 color{1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    std::vector<Vertex> vertices = {
        {{-h, 0.0f, -h}, up, color, {0, 0}},
        {{ h, 0.0f, -h}, up, color, {1, 0}},
        {{ h, 0.0f,  h}, up, color, {1, 1}},
        {{-h, 0.0f,  h}, up, color, {0, 1}},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

    return Mesh{device, vertices, indices};
}

} // namespace mve
