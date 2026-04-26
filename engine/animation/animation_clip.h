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

    const std::string& Name() const { return name_; }
    float Duration() const { return duration_; }

    void SetBoneTrack(uint32_t bone_index, const BoneTrack& track);

    void Sample(
        float time,
        uint32_t bone_count,
        const std::vector<glm::vec3>& default_positions,
        const std::vector<glm::quat>& default_rotations,
        const std::vector<glm::vec3>& default_scales,
        std::vector<glm::vec3>& out_positions,
        std::vector<glm::quat>& out_rotations,
        std::vector<glm::vec3>& out_scales) const;

private:
    // See docs/wiki/Math-Interpolation.md
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
