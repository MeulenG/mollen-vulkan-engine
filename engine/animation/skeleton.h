#ifndef MVE_SKELETON_H
#define MVE_SKELETON_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

namespace mve {

// A single bone in the hierarchy.
// parent_index = -1 means this is a root bone.
struct Bone {
    std::string name;
    int32_t parent_index = -1;

    // Bind pose — the bone's default local transform (in parent space).
    // This is the T-pose or rest pose of the model.
    glm::vec3 bind_position{0.0f};
    glm::quat bind_rotation{1.0f, 0.0f, 0.0f, 0.0f}; // identity quaternion (w,x,y,z)
    glm::vec3 bind_scale{1.0f};
};

class Skeleton {
public:
    Skeleton() = default;

    // Add a bone. Returns its index.
    uint32_t addBone(const Bone& bone);

    uint32_t boneCount() const { return static_cast<uint32_t>(bones_.size()); }
    const Bone& getBone(uint32_t index) const { return bones_[index]; }

    // Compute the inverse bind matrices from the bind pose.
    // Call this ONCE after all bones are added.
    // These transform vertices from model space → bone-local space.
    void computeInverseBindMatrices();

    // Given current local transforms for each bone (from animation),
    // compute the final bone matrices: current_world * inverse_bind.
    // This is what gets uploaded to the GPU.
    std::vector<glm::mat4> computeBoneMatrices(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::quat>& rotations,
        const std::vector<glm::vec3>& scales) const;

    // M2-specific: compute bone matrices using pivot points.
    // M2 bones animate around their pivot: T(pivot) * R * T(anim_translate) * T(-pivot)
    // Then chain through parent hierarchy.
    std::vector<glm::mat4> computeM2BoneMatrices(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::quat>& rotations,
        const std::vector<glm::vec3>& scales) const;

    // Store pivot points for M2 mode
    void setPivot(uint32_t bone_index, glm::vec3 pivot) {
        if (pivots_.size() <= bone_index) pivots_.resize(bone_index + 1, glm::vec3{0.0f});
        pivots_[bone_index] = pivot;
    }
    bool hasM2Pivots() const { return !pivots_.empty(); }

    // Get the bind-pose bone matrices (for static/un-animated rendering)
    std::vector<glm::mat4> getIdentityBoneMatrices() const;

    static constexpr uint32_t MAX_BONES = 256;

private:
    // Compose a local transform matrix from position, rotation, scale.
    //
    // The math: T * R * S where
    //   T = translation matrix (moves the origin)
    //   R = rotation matrix (from quaternion)
    //   S = scale matrix (stretches axes)
    //
    // Applied right-to-left: first scale, then rotate, then translate.
    static glm::mat4 composeTransform(
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3& scale);

    std::vector<Bone> bones_;
    std::vector<glm::mat4> inverse_bind_matrices_;
    std::vector<glm::vec3> pivots_; // M2-specific: absolute model-space pivot per bone
};

} // namespace mve

#endif // MVE_SKELETON_H
