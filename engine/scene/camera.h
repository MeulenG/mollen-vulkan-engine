#ifndef MVE_CAMERA_H
#define MVE_CAMERA_H

#include <glm/glm.hpp>

namespace mve {

class Camera {
public:
    Camera() = default;

private:
    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    glm::vec3 target_{0.0f, 0.0f, 0.0f};
    float fov_ = 45.0f;
    float near_ = 0.1f;
    float far_ = 1000.0f;
};

} // namespace mve

#endif // MVE_CAMERA_H
