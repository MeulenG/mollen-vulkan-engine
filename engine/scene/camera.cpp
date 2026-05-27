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

    if (mode_ == CameraMode::Orbit || mode_ == CameraMode::ThirdPerson) {
        // Orbit + ThirdPerson share view math: camera position is
        // derived from target + yaw/pitch/distance. The difference is
        // who updates target - in Orbit, target is fixed; in
        // ThirdPerson, the player controller drives it each frame.
        updatePosition();
    } else {
        // First-person: position is fixed, recompute target so look
        // direction reflects new yaw/pitch.
        updateTargetFromAngles();
    }
}

void Camera::Pan(float dx, float dy) {
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 right = glm::normalize(glm::cross(forward, up_));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    target_ += right * dx + up * dy;
    if (mode_ == CameraMode::FlyFirstPerson) {
        position_ += right * dx + up * dy;
    }
    if (mode_ == CameraMode::Orbit || mode_ == CameraMode::ThirdPerson) {
        updatePosition();
    }
}

void Camera::Zoom(float delta) {
    if (mode_ == CameraMode::Orbit || mode_ == CameraMode::ThirdPerson) {
        distance_ = std::max(0.1f, distance_ - delta);
        updatePosition();
    } else {
        // First-person: zoom translates forward along look direction.
        Move(0.0f, 0.0f, delta);
    }
}

void Camera::Move(float dx, float dy, float dz) {
    // Translate position + target by camera-local (right, up, forward).
    // Keeps the look direction constant so the user can walk forward
    // while still looking the same way.
    glm::vec3 forward = glm::normalize(target_ - position_);
    glm::vec3 right   = glm::normalize(glm::cross(forward, up_));
    glm::vec3 up      = glm::normalize(glm::cross(right, forward));

    glm::vec3 delta = right * dx + up * dy + forward * dz;
    position_ += delta;
    target_   += delta;
}

void Camera::SetMode(CameraMode m) {
    if (m == mode_) return;
    CameraMode prev = mode_;
    if (m == CameraMode::FlyFirstPerson) {
        // Orbit/ThirdPerson -> FPS. In orbit-style modes yaw/pitch
        // describe the offset from target to camera (we look back
        // from there). In FPS they describe the forward direction.
        // To keep the view continuous through the toggle, derive new
        // yaw/pitch from the current look direction (target - position).
        glm::vec3 fwd = target_ - position_;
        if (glm::length(fwd) > 1e-4f) {
            fwd = glm::normalize(fwd);
            pitch_ = std::asin(glm::clamp(fwd.y, -1.0f, 1.0f));
            yaw_   = std::atan2(fwd.x, fwd.z);
        }
        mode_ = m;
        updateTargetFromAngles();
    } else if (prev == CameraMode::FlyFirstPerson) {
        // FPS -> orbit/ThirdPerson. Snap the orbit center to ~8 units
        // ahead of the camera (so the user can rotate around the area
        // they were looking at). Re-derive orbit yaw/pitch from the
        // inverse of the look direction.
        glm::vec3 fwd = target_ - position_;
        if (glm::length(fwd) < 1e-4f) fwd = glm::vec3{0, 0, 1};
        fwd = glm::normalize(fwd);
        distance_ = 8.0f;
        target_ = position_ + fwd * distance_;
        glm::vec3 back = -fwd;
        pitch_ = std::asin(glm::clamp(back.y, -1.0f, 1.0f));
        yaw_   = std::atan2(back.x, back.z);
        mode_ = m;
        updatePosition();
    } else {
        // Orbit <-> ThirdPerson: same view math, just a label change.
        // Preserve target / yaw / pitch / distance exactly as-is so
        // SetOrbit(...) called right BEFORE SetMode keeps its effect.
        mode_ = m;
        updatePosition();
    }
}

void Camera::updateTargetFromAngles() {
    // Engine is Z-up. yaw rotates around +Z (vertical), pitch tilts
    // up/down. yaw=0 points along +Y (north in our render frame).
    //   horizontal_xy: ( sin yaw, cos yaw )
    //   vertical_z:    sin pitch
    float ch = std::cos(pitch_);
    float x = ch * std::sin(yaw_);
    float y = ch * std::cos(yaw_);
    float z = std::sin(pitch_);
    target_ = position_ + glm::vec3{x, y, z};
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
    // Orbit position in Z-up basis. yaw rotates around +Z; pitch
    // tilts up/down. The orbiter sits BEHIND the target along the
    // look direction, so we offset by +(opposite forward).
    float ch = std::cos(pitch_);
    float x = distance_ * ch * std::sin(yaw_);
    float y = distance_ * ch * std::cos(yaw_);
    float z = distance_ * std::sin(pitch_);
    position_ = target_ + glm::vec3{x, y, z};
}

} // namespace mve
