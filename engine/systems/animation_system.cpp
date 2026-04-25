#include "animation_system.h"
#include "../scene/components/skeleton_component.h"
#include "../scene/components/material_component.h"
#include "../animation/skeleton.h"

namespace mve {

void AnimationSystem::update(Scene& scene, float delta_time) {
    scene.each<SkeletonComponent, MaterialComponent>(
        [&](Entity& entity, SkeletonComponent& skel, MaterialComponent& mat) {
            if (!skel.pm_playing || !skel.pm_animator || !mat.pm_bone_buffer) return;

            skel.pm_animator->update(delta_time * skel.pm_speed);

            // Upload bone matrices to GPU
            auto matrices = skel.pm_animator->BoneMatrices();
            matrices.resize(Skeleton::MAX_BONES, glm::mat4{1.0f});
            vk::DeviceSize size = Skeleton::MAX_BONES * sizeof(glm::mat4);
            mat.pm_bone_buffer->write(matrices.data(), size);
        });
}

} // namespace mve
