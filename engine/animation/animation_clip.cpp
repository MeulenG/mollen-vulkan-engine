#include "animation_clip.h"

namespace mve {

AnimationClip::AnimationClip(const std::string& name, float duration)
    : name_{name}, duration_{duration} {}

void AnimationClip::setBoneTrack(uint32_t bone_index, const BoneTrack& track) {
    tracks_.emplace_back(bone_index, track);
}

void AnimationClip::sample(
    float time,
    uint32_t bone_count,
    const std::vector<glm::vec3>& default_positions,
    const std::vector<glm::quat>& default_rotations,
    const std::vector<glm::vec3>& default_scales,
    std::vector<glm::vec3>& out_positions,
    std::vector<glm::quat>& out_rotations,
    std::vector<glm::vec3>& out_scales) const {

    out_positions = default_positions;
    out_rotations = default_rotations;
    out_scales = default_scales;

    for (const auto& [bone_index, track] : tracks_) {
        if (bone_index >= bone_count) continue;

        if (!track.position_keys.empty()) {
            out_positions[bone_index] = interpolateVec3(
                track.position_keys, time, default_positions[bone_index]);
        }
        if (!track.rotation_keys.empty()) {
            out_rotations[bone_index] = interpolateQuat(
                track.rotation_keys, time, default_rotations[bone_index]);
        }
        if (!track.scale_keys.empty()) {
            out_scales[bone_index] = interpolateVec3(
                track.scale_keys, time, default_scales[bone_index]);
        }
    }
}

// Linear interpolation for vec3 (positions and scales)
// lerp(a, b, t) = a * (1 - t) + b * t
// Geometrically: a straight line from a to b.
glm::vec3 AnimationClip::interpolateVec3(
    const std::vector<Keyframe<glm::vec3>>& keys,
    float time,
    const glm::vec3& default_value) {

    if (keys.empty()) return default_value;
    if (keys.size() == 1) return keys[0].value;
    if (time <= keys.front().timestamp) return keys.front().value;
    if (time >= keys.back().timestamp) return keys.back().value;

    for (size_t i = 0; i < keys.size() - 1; i++) {
        if (time < keys[i + 1].timestamp) {
            float t = (time - keys[i].timestamp) /
                      (keys[i + 1].timestamp - keys[i].timestamp);
            return glm::mix(keys[i].value, keys[i + 1].value, t);
        }
    }
    return keys.back().value;
}

// Spherical linear interpolation for quaternions (rotations)
//
// slerp(q0, q1, t) = q0 * sin((1-t)*theta)/sin(theta) + q1 * sin(t*theta)/sin(theta)
// where theta = acos(|dot(q0, q1)|)
//
// If dot(q0, q1) < 0, negate one quaternion to take the SHORT rotation path.
glm::quat AnimationClip::interpolateQuat(
    const std::vector<Keyframe<glm::quat>>& keys,
    float time,
    const glm::quat& default_value) {

    if (keys.empty()) return default_value;
    if (keys.size() == 1) return keys[0].value;
    if (time <= keys.front().timestamp) return keys.front().value;
    if (time >= keys.back().timestamp) return keys.back().value;

    for (size_t i = 0; i < keys.size() - 1; i++) {
        if (time < keys[i + 1].timestamp) {
            float t = (time - keys[i].timestamp) /
                      (keys[i + 1].timestamp - keys[i].timestamp);

            glm::quat q0 = keys[i].value;
            glm::quat q1 = keys[i + 1].value;

            // Ensure shortest path
            if (glm::dot(q0, q1) < 0.0f) {
                q1 = -q1;
            }

            return glm::slerp(q0, q1, t);
        }
    }
    return keys.back().value;
}

} // namespace mve
