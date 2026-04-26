#ifndef MVE_TRANSFORM_COMPONENT_H
#define MVE_TRANSFORM_COMPONENT_H

#include "../entity.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mve {

struct TransformComponent : Component {
    glm::vec3 pm_position{0.0f};
    glm::quat pm_rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 pm_scale{1.0f};

    glm::mat4 ModelMatrix() const {
        glm::mat4 m = glm::translate(glm::mat4{1.0f}, pm_position);
        m = m * glm::mat4_cast(pm_rotation);
        m = glm::scale(m, pm_scale);
        return m;
    }

    // See docs/wiki/Math-Transforms.md
    void ApplyWowCoordTransform(float ground_offset) {
        pm_position.y = ground_offset;
        pm_rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3{1, 0, 0});
    }
};

} // namespace mve

#endif // MVE_TRANSFORM_COMPONENT_H
