#ifndef MVE_PLAYER_CONTROLLER_H
#define MVE_PLAYER_CONTROLLER_H

#include "camera.h"

#include <glm/glm.hpp>

namespace mve {

// MVP third-person player controller. Owns a world-space player
// position + facing yaw. The main loop drives it each frame:
//
//   controller.Update(dt, cam, ImGui::IsKeyDown(W), ...);
//   cam.SetTarget(controller.GetEyePos());
//
// Movement is "camera-relative" - W moves the player along the
// camera's flat forward direction (projection of cam forward onto the
// XZ plane), A/D strafe along the camera's flat right.
//
// Phase 2B v1: no collision, no height query, no jump. The player
// glides on a fixed Y plane. Phase 2B.2 will add ADT terrain height
// sampling so the player follows the ground.
class PlayerController {
public:
    void SetPosition(const glm::vec3& p) { pm_position = p; }
    glm::vec3 GetPosition() const         { return pm_position; }

    // Camera target = player head height (1.7 yards above feet, which
    // is the WoW player height). Orbit camera looks at this.
    glm::vec3 GetEyePos() const           { return pm_position + glm::vec3{0, 1.7f, 0}; }

    // Tunable walk speed (yards per second). Held-shift sprint
    // multiplier is in the input handler, not here.
    float GetWalkSpeed() const            { return pm_walk_speed; }
    void  SetWalkSpeed(float s)           { pm_walk_speed = s; }

    // Per-frame update. dt is real seconds. forward / right / strafe /
    // up axes are read from the camera so movement is camera-relative.
    // The four input flags are the raw WASD key states - the controller
    // composes them into a unit-length world-space delta.
    void Update(float dt,
                const Camera& cam,
                bool key_w, bool key_a, bool key_s, bool key_d,
                bool sprint);

private:
    glm::vec3 pm_position{0.0f};
    float     pm_walk_speed = 7.0f;   // WoW player run speed ~= 7 yd/s
};

} // namespace mve

#endif // MVE_PLAYER_CONTROLLER_H
