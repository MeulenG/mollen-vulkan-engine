#ifndef MVE_M2_INFO_COMPONENT_H
#define MVE_M2_INFO_COMPONENT_H

#include "../entity.h"
#include "../../formats/m2_loader.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace mve {

struct M2InfoComponent : Component {
    std::string pm_model_name;
    uint32_t pm_vertex_count = 0;
    uint32_t pm_index_count = 0;
    uint32_t pm_bone_count = 0;
    std::vector<std::string> pm_texture_paths;
    std::vector<M2Submesh> pm_submeshes;
    glm::vec3 pm_bbox_min{0.0f};
    glm::vec3 pm_bbox_max{0.0f};
    float pm_ground_offset = 0.0f;
};

} // namespace mve

#endif // MVE_M2_INFO_COMPONENT_H
