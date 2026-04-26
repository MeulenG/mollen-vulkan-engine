#include "camera.h"

#include <algorithm>
#include <cmath>

namespace mve {

void Camera::SetOrbit(float distance, float yaw, float pitch) {
    distance_ = distance;
    yaw_ = yaw;
    pitch_ = pitch;
    updatePosition();
}

void Camera::Rotate(float delta_yaw, float delta_pitch) {
    yaw_ += delta_yaw;
    pitch_ += delta_pitch;

    // Clamp pitch to avoid flipping
    constexpr float limit = glm::radians(89.0f);
    pitch_ = std::clamp(pitch_, -limit, limit);

    updatePosition();
}

void Camera::Pan(float dx, float dy) {
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 right = glm::normalize(glm::cross(forward, up_));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    target_ += right * dx + up * dy;
    updatePosition();
}

void Camera::Zoom(float delta) {
    distance_ = std::max(0.1f, distance_ - delta);
    updatePosition();
}

void Camera::SetPerspective(float fov_degrees, float aspect, float near, float far) {
    projection_ = glm::perspective(glm::radians(fov_degrees), aspect, near, far);
    // Vulkan clip space has inverted Y
    projection_[1][1] *= -1.0f;
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position_, target_, up_);
}

glm::vec3 Camera::GetPosition() const {
    return position_;
}

void Camera::updatePosition() {
    float x = distance_ * std::cos(pitch_) * std::sin(yaw_);
    float y = distance_ * std::sin(pitch_);
    float z = distance_ * std::cos(pitch_) * std::cos(yaw_);

    position_ = target_ + glm::vec3{x, y, z};
}

} // namespace mve
