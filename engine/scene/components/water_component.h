#ifndef MVE_WATER_COMPONENT_H
#define MVE_WATER_COMPONENT_H

#include "../entity.h"
#include "../mesh.h"

#include <memory>
#include <vector>

namespace mve {

// Holds the per-instance water meshes built for one ADT tile. Each
// AdtLiquidInstance produces one Mesh; a typical Elwynn tile yields
// ~5-30 (Crystal Brook fragments, Stormwind moat segments, abbey
// pond). The component lives on the same entity as TerrainComponent
// so tile eviction destroys the water meshes alongside the terrain.
//
// Per-instance liquid_type is captured so a future Tier 2 shader path
// can pick the correct texture set (river/ocean/magma/slime) per draw.
struct WaterInstanceMesh {
    std::unique_ptr<Mesh> mesh;
    uint16_t liquid_type = 0;   // FK -> LiquidType.dbc
};

struct WaterComponent : Component {
    std::vector<WaterInstanceMesh> instances;
};

} // namespace mve

#endif // MVE_WATER_COMPONENT_H
