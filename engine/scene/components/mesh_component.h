#ifndef MVE_MESH_COMPONENT_H
#define MVE_MESH_COMPONENT_H

#include "../mesh.h"

#include <memory>

namespace mve {

struct MeshComponent {
    std::shared_ptr<Mesh> pm_mesh;
    bool pm_visible = true;
};

} // namespace mve

#endif // MVE_MESH_COMPONENT_H
