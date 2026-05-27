#include "player_controller.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace mve {

void PlayerController::Update(float dt,
                               const Camera& cam,
                               bool key_w, bool key_a, bool key_s, bool key_d,
                               bool sprint) {
    // Read camera-relative axes from its current view direction. We
    // FLATTEN forward + right onto the XY plane so pushing W never
    // makes the player walk into the sky (or into the ground) when
    // the camera is angled. The world up is +Z in engine space.
    glm::vec3 cam_fwd = cam.GetTarget() - cam.GetPosition();
    cam_fwd.z = 0.0f;
    float flen = std::sqrt(cam_fwd.x * cam_fwd.x + cam_fwd.y * cam_fwd.y);
    if (flen < 1e-4f) {
        // Camera looking straight down - default forward to +Y (north).
        cam_fwd = glm::vec3{0, 1, 0};
    } else {
        cam_fwd /= flen;
    }
    // Right = forward x up. With up=+Z and fwd=(fx,fy,0):
    // right = fwd x up = (fy, -fx, 0).
    glm::vec3 cam_right{cam_fwd.y, -cam_fwd.x, 0.0f};

    // Compose unit-length world-space movement vector from input.
    glm::vec3 wish{0.0f};
    if (key_w) wish += cam_fwd;
    if (key_s) wish -= cam_fwd;
    if (key_d) wish += cam_right;
    if (key_a) wish -= cam_right;

    float wlen = std::sqrt(wish.x * wish.x + wish.y * wish.y + wish.z * wish.z);
    if (wlen < 1e-4f) return;  // no input

    wish /= wlen;

    // Apply walk speed + sprint multiplier (~5x, matching the editor's
    // FPS-mode sprint feel).
    float speed = pm_walk_speed * (sprint ? 5.0f : 1.0f);
    pm_position += wish * (speed * dt);
}

} // namespace mve
