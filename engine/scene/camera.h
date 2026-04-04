#ifndef MVE_CAMERA_H
#define MVE_CAMERA_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mve {

class Camera {
public:
    Camera() = default;

    void setOrbit(float distance, float yaw, float pitch);
    void setTarget(glm::vec3 target) { target_ = target; }

    void rotate(float delta_yaw, float delta_pitch);
    void pan(float dx, float dy);
    void zoom(float delta);

    void setPerspective(float fov_degrees, float aspect, float near, float far);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const { return projection_; }

    glm::vec3 getPosition() const;

private:
    void updatePosition();

    glm::vec3 target_{0.0f, 0.0f, 0.0f};
    glm::vec3 position_{0.0f, 0.0f, 3.0f};
    glm::vec3 up_{0.0f, 1.0f, 0.0f};

    float distance_ = 3.0f;
    float yaw_ = 0.0f;       // radians
    float pitch_ = 0.3f;     // radians

    glm::mat4 projection_{1.0f};
};

} // namespace mve

#endif // MVE_CAMERA_H
