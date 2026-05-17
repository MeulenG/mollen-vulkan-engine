#ifndef MVE_CAMERA_H
#define MVE_CAMERA_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mve {

// Two modes:
//   Orbit (default): camera rotates around a fixed target at a fixed
//     distance. Used to inspect a model or a fixed location. The R3/R4
//     editor preview uses this on the Northshire tile centroid.
//   FlyFirstPerson: camera position is the primary - rotation rotates
//     where it's looking, WASD-style movement translates it. Used to
//     "walk through" the scene, which makes the trees / ground feel
//     like a real environment instead of a diorama.
enum class CameraMode {
    Orbit,
    FlyFirstPerson,
};

class Camera {
public:
    Camera() = default;

    void SetOrbit(float distance, float yaw, float pitch);
    void SetTarget(glm::vec3 target) { target_ = target; }

    void Rotate(float delta_yaw, float delta_pitch);
    void Pan(float dx, float dy);
    void Zoom(float delta);

    // First-person controls. Move translates along the camera's local
    // axes: dx = strafe (right is +), dy = elevate (up is +),
    // dz = forward (toward look direction is +). Internally repositions
    // both position_ and target_ so the look direction stays constant.
    void Move(float dx, float dy, float dz);

    CameraMode Mode() const { return mode_; }
    void SetMode(CameraMode m);

    void SetPerspective(float fov_degrees, float aspect, float near, float far);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const { return projection_; }

    glm::vec3 GetPosition() const;
    glm::vec3 GetTarget() const { return target_; }
    float Distance() const { return distance_; }

private:
    void updatePosition();
    void updateTargetFromAngles();

    CameraMode mode_ = CameraMode::Orbit;

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
