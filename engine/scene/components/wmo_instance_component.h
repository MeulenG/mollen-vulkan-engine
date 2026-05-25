#ifndef MVE_WMO_INSTANCE_COMPONENT_H
#define MVE_WMO_INSTANCE_COMPONENT_H

#include "../entity.h"
#include "../wmo_mesh.h"
#include "../../formats/wmo_types.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace mve {

// One placed WMO (building). Each ADT MODF entry that survives dedup
// becomes one entity with this component plus a TransformComponent
// carrying the model matrix (T(engine_pos) * R(world_euler), no M2
// axis swap since WMO vertices are already Y-up).
//
// Holds N group GPU meshes + a shared pointer to the WmoRoot for
// material/texture lookup at draw time.
struct WmoInstanceComponent : Component {
    std::shared_ptr<WmoRoot>      root;
    std::vector<WmoGroupGpu>      groups;       // one per group_file
    std::vector<glm::vec4>        group_bbox_min; // WMO-local, .w = unused
    std::vector<glm::vec4>        group_bbox_max;
    glm::mat4                     model_matrix{1.0f};
};

} // namespace mve

#endif // MVE_WMO_INSTANCE_COMPONENT_H
