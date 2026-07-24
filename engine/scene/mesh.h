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

    static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
};

class Mesh {
public:
    // Construct from a vector of the canonical M2/model Vertex struct.
    // Most call sites use this.
    Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    // Construct from a raw byte blob of vertex data. Used by code paths
    // that produce a custom vertex layout (e.g. terrain has its own
    // TerrainVertex with a per-vertex chunk_index, separate from the
    // skinned-model Vertex). The caller is responsible for providing the
    // matching VertexInput{Binding,Attribute}Description list when the
    // mesh is bound to a pipeline.
    Mesh(Device& device, const void* vertex_data, size_t vertex_bytes,
         const std::vector<uint32_t>& indices);

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    void Bind(const vk::raii::CommandBuffer& command_buffer) const;
    void Draw(const vk::raii::CommandBuffer& command_buffer) const;

    // Instanced draw - issues drawIndexed(idx_count, instance_count, 0,
    // 0, 0). The shader reads per-instance data via gl_InstanceIndex.
    // Used by the doodad path (R4.5): one mesh + one descriptor set +
    // one draw call covers N placements of the same M2.
    void DrawInstanced(const vk::raii::CommandBuffer& command_buffer,
                       uint32_t instance_count) const;

    // Instanced draw of a sub-range of the index buffer. Used by R4.8
    // per-submesh M2 rendering: one mesh covers the whole M2, each
    // submesh is a [index_start, index_count] range. Different submeshes
    // bind different descriptor sets so each can carry its own texture.
    void DrawInstancedRange(const vk::raii::CommandBuffer& command_buffer,
                            uint32_t instance_count,
                            uint32_t index_start,
                            uint32_t index_count) const;

    uint32_t IndexCount() const { return index_count_; }

    static Mesh CreatePyramid(Device& device, glm::vec3 color = {0.8f, 0.8f, 0.8f});
    static Mesh CreateCube(Device& device, glm::vec3 color = {0.8f, 0.8f, 0.8f});
    static Mesh CreateGroundPlane(Device& device, float size = 20.0f);

private:
    uint32_t index_count_;
    std::unique_ptr<Buffer> vertex_buffer_;
    std::unique_ptr<Buffer> index_buffer_;
};

} // namespace mve

#endif // MVE_MESH_H
