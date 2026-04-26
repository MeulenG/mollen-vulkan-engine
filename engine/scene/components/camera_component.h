#ifndef MVE_CAMERA_COMPONENT_H
#define MVE_CAMERA_COMPONENT_H

#include "../entity.h"
#include "../camera.h"

namespace mve {

struct CameraComponent : Component {
    Camera pm_camera;
    bool pm_is_active = false;
    float pm_fov = 45.0f;
    float pm_near = 0.1f;
    float pm_far = 1000.0f;
};

} // namespace mve

#endif // MVE_CAMERA_COMPONENT_H
