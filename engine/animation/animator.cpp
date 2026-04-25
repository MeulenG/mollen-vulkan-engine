#include "animator.h"

#include <cmath>

namespace mve {

Animator::Animator(const Skeleton& skeleton) : skeleton_{skeleton} {
    uint32_t count = skeleton_.BoneCount();

    // Cache the bind pose defaults
    bind_positions_.resize(count);
    bind_rotations_.resize(count);
    bind_scales_.resize(count);

    for (uint32_t i = 0; i < count; i++) {
        const auto& bone = skeleton_.GetBone(i);
        bind_positions_[i] = bone.bind_position;
        bind_rotations_[i] = bone.bind_rotation;
        bind_scales_[i] = bone.bind_scale;
    }

    // Start with identity matrices (bind pose)
    bone_matrices_ = skeleton_.GetIdentityBoneMatrices();
}

void Animator::play(const AnimationClip* clip, bool loop) {
    current_clip_ = clip;
    playing_ = true;
    looping_ = loop;
    current_time_ = 0.0f;
}

void Animator::stop() {
    playing_ = false;
    current_time_ = 0.0f;
    bone_matrices_ = skeleton_.GetIdentityBoneMatrices();
}

void Animator::update(float delta_time) {
    if (!playing_ || !current_clip_) return;

    current_time_ += delta_time;

    if (current_time_ >= current_clip_->duration()) {
        if (looping_) {
            // fmod wraps the time back to the start
            // e.g., if duration=2.0 and time=2.3, we get 0.3
            current_time_ = std::fmod(current_time_, current_clip_->duration());
        } else {
            current_time_ = current_clip_->duration();
            playing_ = false;
        }
    }

    // Sample the animation at current time to get per-bone transforms
    std::vector<glm::vec3> positions;
    std::vector<glm::quat> rotations;
    std::vector<glm::vec3> scales;

    current_clip_->sample(
        current_time_,
        skeleton_.BoneCount(),
        bind_positions_,
        bind_rotations_,
        bind_scales_,
        positions,
        rotations,
        scales);

    // Use M2 pivot-based computation if available, otherwise generic
    if (skeleton_.HasM2Pivots()) {
        bone_matrices_ = skeleton_.ComputeM2BoneMatrices(positions, rotations, scales);
    } else {
        bone_matrices_ = skeleton_.ComputeBoneMatrices(positions, rotations, scales);
    }
}

} // namespace mve
