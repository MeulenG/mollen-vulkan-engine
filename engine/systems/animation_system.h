#ifndef MVE_ANIMATION_SYSTEM_H
#define MVE_ANIMATION_SYSTEM_H

#include "../scene/scene.h"

namespace mve {

class AnimationSystem {
public:
    void update(Scene& scene, float delta_time);
};

} // namespace mve

#endif // MVE_ANIMATION_SYSTEM_H
