#ifndef MVE_TERRAIN_TILE_COMPONENT_H
#define MVE_TERRAIN_TILE_COMPONENT_H


#include <glm/glm.hpp>

namespace mve {

// Identifies a loaded terrain entity as a specific ADT tile. The
// streamer queries Each<TerrainTileComponent> instead of maintaining a
// parallel map - keeps a single source of truth (the scene) and makes
// the editor UI introspectable.
//
// pm_centroid_engine is the tile's geometric center in engine space.
// Used by the streamer for distance-based eviction decisions and by
// future camera-snap features.
struct TerrainTileComponent {
    int pm_tile_x = 0;             // 0..63 along WoW X axis (south)
    int pm_tile_y = 0;             // 0..63 along WoW Y axis (east)
    glm::vec3 pm_centroid_engine{0.0f};
    float pm_radius = 0.0f;        // ~377 yards (half-diagonal)
};

} // namespace mve

#endif // MVE_TERRAIN_TILE_COMPONENT_H
