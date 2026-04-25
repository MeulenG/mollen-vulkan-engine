#ifndef MVE_SKELETON_COMPONENT_H
#define MVE_SKELETON_COMPONENT_H

#include "../entity.h"
#include "../../animation/skeleton.h"
#include "../../animation/animation_clip.h"
#include "../../animation/animator.h"

#include <memory>
#include <vector>

namespace mve {

struct SkeletonComponent : Component {
    const Skeleton* pm_skeleton = nullptr;
    std::unique_ptr<Animator> pm_animator;
    std::vector<const AnimationClip*> pm_clips;
    int pm_current_clip_index = 0;
    bool pm_playing = true;
    float pm_speed = 1.0f;
};

} // namespace mve

#endif // MVE_SKELETON_COMPONENT_H
