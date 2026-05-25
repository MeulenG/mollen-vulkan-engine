#ifndef MVE_WMO_MESH_H
#define MVE_WMO_MESH_H

#include "mesh.h"
#include "../formats/wmo_types.h"

#include <memory>
#include <vector>

namespace mve {

// Per-vertex layout for WMO group geometry. Distinct from terrain /
// M2 / water vertex formats so the WMO pipeline carries only what
// WoW's WMO shaders need:
//   location 0  vec3 position    - WMO-local frame
//   location 1  vec3 normal      - WMO-local frame
//   location 2  vec2 uv1         - texture coord layer 1
//   location 3  vec2 uv2         - texture coord layer 2 (TVerts2)
//   location 4  vec4 color1      - MOCV1 baked light + int/ext alpha
//   location 5  vec4 color2      - MOCV2 blend mask (TwoLayer shaders)
struct WmoVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv1;
    glm::vec2 uv2;
    glm::vec4 color1;
    glm::vec4 color2;

    static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
};

// Per-batch metadata captured at mesh-build time. One entry per WMO
// MOBA, in the same order the batches appear in the parsed group
// file (trans -> interior -> exterior). The render system iterates
// these alongside the root's materials to pick blend mode, shader,
// and the diffuse texture for each draw.
struct WmoBatchDrawInfo {
    uint32_t first_index  = 0;
    uint32_t num_indices  = 0;
    uint8_t  material_id  = 0;
    uint8_t  is_interior  = 0;   // 1 if this batch came from int_batch_count
    uint8_t  is_trans     = 0;   // 1 if from trans_batch_count
};

// One built mesh + its batch list. Lives inside WmoInstanceComponent
// alongside a parent pointer to the shared WmoRoot.
struct WmoGroupGpu {
    std::unique_ptr<Mesh>       mesh;
    std::vector<WmoBatchDrawInfo> batches;
    uint32_t                    group_flags = 0;   // copy of MOGP.flags
};

class WmoMesh {
public:
    // Build a GPU mesh from one parsed WmoGroup. Vertices are kept in
    // WMO-LOCAL coordinates (no axis swap applied here); the spawn
    // code's model matrix handles the basis change to engine frame.
    // Returns an empty WmoGroupGpu when the group has no renderable
    // batches (e.g. ANTIPORTAL groups with no MOBA entries).
    static WmoGroupGpu Build(Device& device, const WmoGroup& group);
};

} // namespace mve

#endif // MVE_WMO_MESH_H
