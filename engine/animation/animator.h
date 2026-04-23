#ifndef MVE_ANIMATOR_H
#define MVE_ANIMATOR_H

#include "skeleton.h"
#include "animation_clip.h"

#include <vector>

namespace mve {

class Animator {
public:
    Animator(const Skeleton& skeleton);

    void play(const AnimationClip* clip, bool loop = true);
    void stop();

    // Advance time and compute bone matrices for this frame.
    // delta_time is in seconds (e.g., 0.016 for 60fps).
    void update(float delta_time);

    bool isPlaying() const { return playing_; }
    float currentTime() const { return current_time_; }

    // The bone matrices to upload to the GPU.
    // Each matrix = current_world * inverse_bind for that bone.
    const std::vector<glm::mat4>& boneMatrices() const { return bone_matrices_; }

private:
    const Skeleton& skeleton_;
    const AnimationClip* current_clip_ = nullptr;
    bool playing_ = false;
    bool looping_ = true;
    float current_time_ = 0.0f;

    // Bind pose defaults (used when a bone has no animation track)
    std::vector<glm::vec3> bind_positions_;
    std::vector<glm::quat> bind_rotations_;
    std::vector<glm::vec3> bind_scales_;

    // Output: final bone matrices for GPU upload
    std::vector<glm::mat4> bone_matrices_;
};

} // namespace mve

#endif // MVE_ANIMATOR_H
