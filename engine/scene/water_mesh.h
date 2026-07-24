#ifndef MVE_WATER_MESH_H
#define MVE_WATER_MESH_H

#include "mesh.h"
#include "../formats/adt_types.h"

#include <memory>
#include <vector>

namespace mve {

// One vertex of a liquid surface. Matches the wow.export liquid vertex
// format - position (engine space, post-WoW-remap), planar UV scaled by
// 0.06 (texture-repeats-every-16.67-yards, per wowdev.wiki), and a
// per-vertex depth in [0,1] for the shallow/deep alpha blend.
struct WaterVertex {
    glm::vec3 position;   // 12 B
    glm::vec2 uv;         //  8 B
    float     depth;      //  4 B  - total 24 B per vertex

    static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions();
    static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
};

// Build one Mesh per AdtLiquidInstance. The instance carries the MCNK
// it's anchored to (chunk_index), the (x_offset, y_offset, width,
// height) cell rectangle within that MCNK, a per-vertex height grid,
// per-vertex depths, and an exists bitmap that gates which cells emit
// triangles (so rivers can carve diagonal banks).
//
// Returns nullptr when the instance has no renderable cells (the
// exists bitmap excludes all of them, or width/height is 0).
class WaterMesh {
public:
    static std::unique_ptr<Mesh> Build(Device& device,
                                        const AdtTile& tile,
                                        const AdtLiquidInstance& inst);
};

} // namespace mve

#endif // MVE_WATER_MESH_H
