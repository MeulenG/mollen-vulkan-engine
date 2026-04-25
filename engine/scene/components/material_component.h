#ifndef MVE_MATERIAL_COMPONENT_H
#define MVE_MATERIAL_COMPONENT_H

#include "../entity.h"
#include "../../resources/buffer.h"
#include "../../resources/image.h"
#include "../../formats/m2_types.h"

#include <memory>
#include <vector>

namespace mve {

struct SubmeshMaterial {
    std::shared_ptr<Image> pm_texture;
    uint16_t pm_blend_mode = 0;
    uint16_t pm_render_flags = 0;
};

struct MaterialComponent : Component {
    std::vector<SubmeshMaterial> pm_submesh_materials;
    vk::DescriptorSet pm_descriptor_set = nullptr;
    std::unique_ptr<Buffer> pm_bone_buffer;
};

} // namespace mve

#endif // MVE_MATERIAL_COMPONENT_H
