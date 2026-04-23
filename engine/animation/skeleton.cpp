#include "skeleton.h"

#include <glm/gtc/matrix_transform.hpp>

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
    // T * R * S
    //
    // glm::translate creates:
    //   [1 0 0 px]
    //   [0 1 0 py]
    //   [0 0 1 pz]
    //   [0 0 0  1]
    //
    // glm::mat4_cast(q) converts quaternion to 3x3 rotation matrix:
    //   The quaternion q = (w, x, y, z) encodes a rotation of
    //   angle = 2*acos(w) around axis = (x, y, z) / sin(acos(w))
    //
    // glm::scale creates:
    //   [sx 0  0  0]
    //   [0  sy 0  0]
    //   [0  0  sz 0]
    //   [0  0  0  1]
    //
    // Multiplied together: first scale the vertex, then rotate it,
    // then translate it. This is the standard TRS decomposition.
    glm::mat4 T = glm::translate(glm::mat4{1.0f}, position);
    glm::mat4 R = glm::mat4_cast(rotation);
    glm::mat4 S = glm::scale(glm::mat4{1.0f}, scale);

    return T * R * S;
}

void Skeleton::computeInverseBindMatrices() {
    uint32_t count = boneCount();

    // First, compute each bone's WORLD-space bind transform
    // by chaining local transforms up the parent hierarchy.
    //
    // For bone i with parent p:
    //   world[i] = world[p] * local[i]
    //
    // For root bones (parent == -1):
    //   world[i] = local[i]
    //
    // This works because we add bones in order: parents before children.
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

    // The inverse bind matrix transforms a vertex from model space
    // into bone-local space. This is needed because:
    //
    // skinned_pos = bone_world * inverse_bind * model_pos
    //            = bone_world * (bind_world^-1) * model_pos
    //
    // When the bone is in bind pose: bone_world == bind_world,
    // so the result is: bind_world * bind_world^-1 * pos = pos
    // (identity — the mesh stays in its original shape)
    //
    // When the bone moves, the difference from bind pose deforms the mesh.
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

    // Same parent-chain logic as bind pose, but using animated transforms
    for (uint32_t i = 0; i < count; i++) {
        glm::mat4 local = composeTransform(positions[i], rotations[i], scales[i]);

        if (bones_[i].parent_index >= 0) {
            world_transforms[i] = world_transforms[bones_[i].parent_index] * local;
        } else {
            world_transforms[i] = local;
        }

        // Final bone matrix = current_world * inverse_bind
        // This is what the vertex shader uses to deform vertices
        bone_matrices[i] = world_transforms[i] * inverse_bind_matrices_[i];
    }

    return bone_matrices;
}

std::vector<glm::mat4> Skeleton::computeM2BoneMatrices(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::quat>& rotations,
    const std::vector<glm::vec3>& scales) const {
    //
    // M2 bone animation:
    //
    // Each bone has a pivot point P in model space. The animated local transform is:
    //   local = T(P + anim_trans) * R(anim_rot) * S(anim_scale) * T(-P)
    //
    // World transform chains through parent:
    //   world[i] = world[parent] * local[i]
    //
    // But vertices are in model space (bind pose). We need to subtract out
    // the bind pose world transform so that at rest, bone_matrix = identity.
    //
    //   bind_local = T(P) * T(-P) = I  (no animation = identity)
    //   bind_world[i] = bind_world[parent] * I = bind_world[parent]
    //
    // For M2, bind_world is just identity for all bones (since T(P)*T(-P)=I
    // chains to identity). So the final matrix IS the world transform.
    //
    // BUT: the issue is that animation keyframes at time=0 may not produce
    // identity — the animation might start in a specific pose that differs
    // from the bind/T-pose. This is expected! The bind pose (all identity)
    // is the raw vertex positions, and animation 0 frame 0 is the first
    // animated pose.
    //
    // The real problem was accumulation through deep chains. Let me verify
    // the formula is correct by ensuring rest pose = identity:
    //   rest: positions[i] = {0,0,0}, rotations[i] = identity, scales[i] = {1,1,1}
    //   local = T(P + 0) * I * I * T(-P) = T(P) * T(-P) = I  ✓

    // From WoWModelViewer Bone::calcMatrix():
    //
    // For each bone:
    //   m = T(pivot)                    // start at pivot position
    //   m = m * T(anim_translation)     // apply animated offset
    //   m = m * R(anim_rotation)        // apply animated rotation
    //   m = m * S(anim_scale)           // apply animated scale
    //   m = m * T(-pivot)               // undo pivot to return to model space
    //
    //   if has parent:
    //     mat = parent.mat * m          // chain through parent
    //   else:
    //     mat = m
    //
    // The pivot is ABSOLUTE (model space), and the same pivot is used
    // for both the initial translation and its negation. The parent
    // chaining handles coordinate space conversion implicitly.

    uint32_t count = boneCount();
    std::vector<glm::mat4> bone_matrices(count, glm::mat4{1.0f});

    for (uint32_t i = 0; i < count; i++) {
        glm::vec3 pivot = (i < pivots_.size()) ? pivots_[i] : glm::vec3{0.0f};

        // Build local matrix: T(pivot) * T(trans) * R(rot) * S(scale) * T(-pivot)
        glm::mat4 m{1.0f};
        m = glm::translate(m, pivot);
        m = glm::translate(m, positions[i]);
        m = m * glm::mat4_cast(rotations[i]);
        m = glm::scale(m, scales[i]);
        m = glm::translate(m, -pivot);

        // Chain through parent
        if (bones_[i].parent_index >= 0) {
            bone_matrices[i] = bone_matrices[bones_[i].parent_index] * m;
        } else {
            bone_matrices[i] = m;
        }
    }

    return bone_matrices;
}

std::vector<glm::mat4> Skeleton::getIdentityBoneMatrices() const {
    // When no animation is playing, return identity matrices
    // (mesh renders in its original bind pose)
    return std::vector<glm::mat4>(boneCount(), glm::mat4{1.0f});
}

} // namespace mve
