#ifndef MVE_WMO_INSTANCE_COMPONENT_H
#define MVE_WMO_INSTANCE_COMPONENT_H

#include "../entity.h"
#include "../mesh.h"
#include "../wmo_mesh.h"
#include "../../formats/wmo_types.h"
#include "../../resources/image.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>

namespace mve {

// One placed WMO (building). Each ADT MODF entry that survives dedup
// becomes one entity with this component plus a TransformComponent
// carrying the model matrix (T(engine_pos) * R(world_euler), no M2
// axis swap since WMO vertices are already Y-up).
//
// Holds N group GPU meshes + a shared pointer to the WmoRoot for
// material/texture lookup at draw time.
struct WmoInstanceComponent : Component {
    std::shared_ptr<WmoRoot>      root;
    std::vector<WmoGroupGpu>      groups;       // one per group_file
    std::vector<glm::vec4>        group_bbox_min; // WMO-local, .w = unused
    std::vector<glm::vec4>        group_bbox_max;
    glm::mat4                     model_matrix{1.0f};

    // Raw MODF data preserved so the render system can rebuild
    // model_matrix per frame when the editor changes WMO debug tuning
    // (yaw offset, sign flips, swap matrix). Without this we'd need
    // a full WMO reload every time a slider moves.
    glm::vec3                     raw_engine_pos{0.0f};
    glm::vec3                     raw_rot_deg{0.0f};

    // Per-material descriptor sets, indexed by MOMT slot. Each set
    // has SceneUBO at binding 0 and the material's diffuse sampler at
    // binding 1. NULL handle = missing texture (the draw loop skips or
    // falls back to material 0).
    std::vector<vk::DescriptorSet>      material_sets;
    // Texture handles kept alive alongside material_sets. The pool's
    // descriptor sets only hold raw vk::Image / vk::Sampler handles -
    // we have to retain the ImageView+Sampler+memory through these
    // shared_ptrs so they don't free while bound.
    std::vector<std::shared_ptr<Image>> material_textures;
};

} // namespace mve

#endif // MVE_WMO_INSTANCE_COMPONENT_H
