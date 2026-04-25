#include "skeleton.h"

#include <glm/gtc/matrix_transform.hpp>

// See docs/wiki/Math-Skeletal-Animation.md
// See docs/wiki/Math-Transforms.md

namespace mve {

uint32_t Skeleton::addBone(const Bone& bone) {
    uint32_t index = static_cast<uint32_t>(bones_.size());
    bones_.push_back(bone);
    return index;
}

glm::mat4 Skeleton::composeTransform(
    const glm::vec3& position,
    const glm::quat& rotation,
    const glm::vec3& scale) {

    glm::mat4 T = glm::translate(glm::mat4{1.0f}, position);
    glm::mat4 R = glm::mat4_cast(rotation);
    glm::mat4 S = glm::scale(glm::mat4{1.0f}, scale);

    return T * R * S;
}

void Skeleton::computeInverseBindMatrices() {
    uint32_t count = boneCount();

    std::vector<glm::mat4> bind_world(count);
    for (uint32_t i = 0; i < count; i++) {
        glm::mat4 local = composeTransform(
            bones_[i].bind_position,
            bones_[i].bind_rotation,
            bones_[i].bind_scale);

        if (bones_[i].parent_index >= 0) {
            bind_world[i] = bind_world[bones_[i].parent_index] * local;
        } else {
            bind_world[i] = local;
        }
    }

    inverse_bind_matrices_.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        inverse_bind_matrices_[i] = glm::inverse(bind_world[i]);
    }
}

std::vector<glm::mat4> Skeleton::computeBoneMatrices(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::quat>& rotations,
    const std::vector<glm::vec3>& scales) const {

    uint32_t count = boneCount();
    std::vector<glm::mat4> world_transforms(count);
    std::vector<glm::mat4> bone_matrices(count);

    for (uint32_t i = 0; i < count; i++) {
        glm::mat4 local = composeTransform(positions[i], rotations[i], scales[i]);

        if (bones_[i].parent_index >= 0) {
            world_transforms[i] = world_transforms[bones_[i].parent_index] * local;
        } else {
            world_transforms[i] = local;
        }

        bone_matrices[i] = world_transforms[i] * inverse_bind_matrices_[i];
    }

    return bone_matrices;
}

std::vector<glm::mat4> Skeleton::computeM2BoneMatrices(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::quat>& rotations,
    const std::vector<glm::vec3>& scales) const {

    uint32_t count = boneCount();
    std::vector<glm::mat4> bone_matrices(count, glm::mat4{1.0f});

    for (uint32_t i = 0; i < count; i++) {
        glm::vec3 pivot = (i < pivots_.size()) ? pivots_[i] : glm::vec3{0.0f};

        glm::mat4 m{1.0f};
        m = glm::translate(m, pivot);
        m = glm::translate(m, positions[i]);
        m = m * glm::mat4_cast(rotations[i]);
        m = glm::scale(m, scales[i]);
        m = glm::translate(m, -pivot);

        if (bones_[i].parent_index >= 0) {
            bone_matrices[i] = bone_matrices[bones_[i].parent_index] * m;
        } else {
            bone_matrices[i] = m;
        }
    }

    return bone_matrices;
}

std::vector<glm::mat4> Skeleton::getIdentityBoneMatrices() const {
    return std::vector<glm::mat4>(boneCount(), glm::mat4{1.0f});
}

} // namespace mve
