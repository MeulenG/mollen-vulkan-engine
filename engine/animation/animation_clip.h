#ifndef MVE_ANIMATION_CLIP_H
#define MVE_ANIMATION_CLIP_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

namespace mve {

// A single keyframe for one property (position, rotation, or scale).
// Timestamp is in seconds.
template<typename T>
struct Keyframe {
    float timestamp;
    T value;
};

// All keyframe tracks for a single bone within one animation.
struct BoneTrack {
    std::vector<Keyframe<glm::vec3>> position_keys;
    std::vector<Keyframe<glm::quat>> rotation_keys;
    std::vector<Keyframe<glm::vec3>> scale_keys;
};

class AnimationClip {
public:
    AnimationClip(const std::string& name, float duration);

    const std::string& name() const { return name_; }
    float duration() const { return duration_; }

    // Set keyframes for a specific bone
    void setBoneTrack(uint32_t bone_index, const BoneTrack& track);

    // Sample the animation at a given time (in seconds).
    // Returns interpolated local transforms for each bone.
    // For bones without keyframes, returns the provided default values.
    void sample(
        float time,
        uint32_t bone_count,
        const std::vector<glm::vec3>& default_positions,
        const std::vector<glm::quat>& default_rotations,
        const std::vector<glm::vec3>& default_scales,
        std::vector<glm::vec3>& out_positions,
        std::vector<glm::quat>& out_rotations,
        std::vector<glm::vec3>& out_scales) const;

private:
    // Interpolate between keyframes at the given time.
    //
    // For positions/scales: linear interpolation (lerp)
    //   result = a * (1 - t) + b * t
    //   Simply moves in a straight line between two values.
    //
    // For rotations: spherical linear interpolation (slerp)
    //   result = q0 * sin((1-t)*θ)/sin(θ) + q1 * sin(t*θ)/sin(θ)
    //   where θ = acos(dot(q0, q1))
    //   This interpolates along the surface of a 4D unit sphere,
    //   giving the shortest, constant-speed rotation between orientations.
    //   GLM's glm::slerp handles this for us.
    static glm::vec3 interpolateVec3(
        const std::vector<Keyframe<glm::vec3>>& keys,
        float time,
        const glm::vec3& default_value);

    static glm::quat interpolateQuat(
        const std::vector<Keyframe<glm::quat>>& keys,
        float time,
        const glm::quat& default_value);

    std::string name_;
    float duration_;
    std::vector<std::pair<uint32_t, BoneTrack>> tracks_; // (bone_index, track)
};

} // namespace mve

#endif // MVE_ANIMATION_CLIP_H
