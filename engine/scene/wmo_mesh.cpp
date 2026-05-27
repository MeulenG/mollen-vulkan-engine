#include "wmo_mesh.h"

namespace mve {

namespace {

// Unpack a WoW BGRA uint32 vertex color to a normalized vec4.
// Channel order in the file: byte0=B, byte1=G, byte2=R, byte3=A.
inline glm::vec4 UnpackBgra(uint32_t bgra) {
    return glm::vec4{
        static_cast<float>((bgra >> 16) & 0xFFu) / 255.0f,
        static_cast<float>((bgra >>  8) & 0xFFu) / 255.0f,
        static_cast<float>((bgra >>  0) & 0xFFu) / 255.0f,
        static_cast<float>((bgra >> 24) & 0xFFu) / 255.0f,
    };
}

} // namespace

std::vector<vk::VertexInputBindingDescription>
WmoVertex::GetBindingDescriptions() {
    return {{0, sizeof(WmoVertex), vk::VertexInputRate::eVertex}};
}

std::vector<vk::VertexInputAttributeDescription>
WmoVertex::GetAttributeDescriptions() {
    return {
        {0, 0, vk::Format::eR32G32B32Sfloat,    offsetof(WmoVertex, position)},
        {1, 0, vk::Format::eR32G32B32Sfloat,    offsetof(WmoVertex, normal)},
        {2, 0, vk::Format::eR32G32Sfloat,       offsetof(WmoVertex, uv1)},
        {3, 0, vk::Format::eR32G32Sfloat,       offsetof(WmoVertex, uv2)},
        {4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(WmoVertex, color1)},
        {5, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(WmoVertex, color2)},
    };
}

WmoGroupGpu WmoMesh::Build(Device& device, const WmoGroup& g) {
    WmoGroupGpu out{};
    out.group_flags = g.header.flags;

    if (g.positions.empty() || g.indices.empty() || g.batches.empty()) {
        return out;
    }

    const size_t n_verts = g.positions.size();
    std::vector<WmoVertex> verts;
    verts.reserve(n_verts);

    // Default color1 is "fully exterior, neutral brightness" when MOCV
    // is absent - WoW's client convention. White alpha 255 means the
    // shader treats it as a pure exterior batch and skips the MOHD
    // ambient term.
    const glm::vec4 default_c1{1.0f, 1.0f, 1.0f, 1.0f};
    const glm::vec4 default_c2{1.0f, 1.0f, 1.0f, 1.0f};

    for (size_t i = 0; i < n_verts; ++i) {
        WmoVertex v{};
        v.position = g.positions[i];
        v.normal   = (i < g.normals.size()) ? g.normals[i] : glm::vec3{0, 0, 1};
        v.uv1      = (i < g.uvs1.size())    ? g.uvs1[i]    : glm::vec2{0, 0};
        v.uv2      = (g.has_uvs2 && i < g.uvs2.size())
                         ? g.uvs2[i] : glm::vec2{0, 0};
        v.color1   = (g.has_colors1 && i < g.colors1.size())
                         ? UnpackBgra(g.colors1[i]) : default_c1;
        v.color2   = (g.has_colors2 && i < g.colors2.size())
                         ? UnpackBgra(g.colors2[i]) : default_c2;
        verts.push_back(v);
    }

    std::vector<uint32_t> indices32;
    indices32.reserve(g.indices.size());
    for (uint16_t idx : g.indices) {
        indices32.push_back(static_cast<uint32_t>(idx));
    }

    out.mesh = std::make_unique<Mesh>(
        device,
        verts.data(),
        verts.size() * sizeof(WmoVertex),
        indices32);

    // Build the batch list. Order in g.batches is trans then interior
    // then exterior (per MOGP layout); capture the type per batch so
    // the shader knows which lighting term to apply.
    out.batches.reserve(g.batches.size());
    uint16_t trans_end = g.header.trans_batch_count;
    uint16_t int_end   = trans_end + g.header.int_batch_count;
    for (uint16_t bi = 0; bi < g.batches.size(); ++bi) {
        const auto& b = g.batches[bi];
        WmoBatchDrawInfo info{};
        info.first_index = b.first_index;
        info.num_indices = b.num_indices;
        info.material_id = b.material_id;
        info.is_trans    = (bi < trans_end) ? 1u : 0u;
        info.is_interior = (bi >= trans_end && bi < int_end) ? 1u : 0u;
        out.batches.push_back(info);
    }
    return out;
}

} // namespace mve
