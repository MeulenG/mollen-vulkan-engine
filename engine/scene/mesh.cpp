#include "mesh.h"

namespace mve {

std::vector<vk::VertexInputBindingDescription> Vertex::getBindingDescriptions() {
    return {{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};
}

std::vector<vk::VertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
        {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
        {2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)},
    };
}

Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : index_count_{static_cast<uint32_t>(indices.size())} {

    vertex_buffer_ = std::make_unique<Buffer>(Buffer::createWithStaging(
        device,
        vertices.data(),
        sizeof(Vertex) * vertices.size(),
        vk::BufferUsageFlagBits::eVertexBuffer));

    index_buffer_ = std::make_unique<Buffer>(Buffer::createWithStaging(
        device,
        indices.data(),
        sizeof(uint32_t) * indices.size(),
        vk::BufferUsageFlagBits::eIndexBuffer));
}

void Mesh::bind(const vk::raii::CommandBuffer& command_buffer) const {
    command_buffer.bindVertexBuffers(0, *vertex_buffer_->buffer(), {0});
    command_buffer.bindIndexBuffer(*index_buffer_->buffer(), 0, vk::IndexType::eUint32);
}

void Mesh::draw(const vk::raii::CommandBuffer& command_buffer) const {
    command_buffer.drawIndexed(index_count_, 1, 0, 0, 0);
}

Mesh Mesh::createCube(Device& device, glm::vec3 color) {
    // Each face has its own vertices for correct normals
    std::vector<Vertex> vertices = {
        // Front face (z = +0.5)
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, color},
        // Back face (z = -0.5)
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, color},
        // Top face (y = +0.5)
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, color},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, color},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, color},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, color},
        // Bottom face (y = -0.5)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, color},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, color},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, color},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, color},
        // Right face (x = +0.5)
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, color},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, color},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, color},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, color},
        // Left face (x = -0.5)
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, color},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, color},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, color},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, color},
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

} // namespace mve
