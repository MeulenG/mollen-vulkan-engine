#ifndef MVE_TERRAIN_STREAMER_H
#define MVE_TERRAIN_STREAMER_H

#include "scene.h"
#include "../resources/asset_manager.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace mve {

// Owns the lifecycle of terrain tile entities in a Scene. Tracks which
// (x, y) slots are loaded, loads new ones as the camera moves, and
// evicts distant ones once they fall outside an outer hysteresis ring.
//
// One TerrainStreamer per Scene. Pure ECS layer - holds no GPU state of
// its own; tile entities live in the Scene, the streamer just sees them
// via Each<TerrainTileComponent> and queues loads through AssetManager.
//
// Coordinate convention (recap):
//   WoW: X=south, Y=east, Z=up
//   Engine: X=east, Y=up, Z=south
//   tile_x increases southward (engine.z direction)
//   tile_y increases eastward (engine.x direction)
//
// EngineToTile derivation:
//   wow_x = engine.z
//   wow_y = engine.x
//   tile_x = 32 - wow_x / 533.33   (because tile 32 is at wow_x=0)
//   tile_y = 32 - wow_y / 533.33
class TerrainStreamer {
public:
    TerrainStreamer(AssetManager& assets, Scene& scene);

    // Load the world's WDT (e.g. "World/Maps/Azeroth/Azeroth.wdt"). The
    // MAIN chunk gives a 64x64 bitmap of which tile slots exist - we
    // skip ocean slots so the streamer doesn't waste IO trying to load
    // missing files.
    bool LoadWdt(const std::string& wow_wdt_path);

    // Synchronously load every tile in a (2r+1) x (2r+1) square around
    // (cx, cy). Used at startup to prime the cache before the first
    // frame, and on demand when the user teleports the camera.
    void PreloadAround(int cx, int cy, int r,
                       const vk::raii::DescriptorSetLayout& terrain_layout);

    // Per-frame update. Computes which tile the camera is currently in,
    // loads any missing tiles in the inner ring, evicts tiles in the
    // outer ring. Cheap when the camera stays in one tile (early-out on
    // unchanged center) and pays one tile of work per boundary crossing.
    void Update(const glm::vec3& camera_pos_engine,
                const vk::raii::DescriptorSetLayout& terrain_layout);

    void SetRadius(int r)       { pm_radius = r; }
    void SetEvictRadius(int r)  { pm_evict_radius = r; }
    int  Radius() const         { return pm_radius; }
    int  EvictRadius() const    { return pm_evict_radius; }

    // True if the WDT MAIN bitmap says tile (x, y) exists on disk.
    bool TileExists(int tile_x, int tile_y) const;

    // Convert an engine-space position to its containing tile indices.
    // Clamped to [0, 63].
    void EngineToTile(const glm::vec3& engine_pos,
                      int& tile_x, int& tile_y) const;

private:
    // Whether (x, y) currently has a TerrainTile entity in the scene.
    bool TilePresent(int tile_x, int tile_y) const;

    AssetManager& pm_assets;
    Scene& pm_scene;

    // 64*64 byte bitmap. 1 = tile exists (per WDT MAIN), 0 = ocean / missing.
    std::vector<uint8_t> pm_tile_exists;
    bool pm_wdt_loaded = false;

    int pm_radius = 1;          // inner ring (load)
    int pm_evict_radius = 2;    // outer ring (evict beyond this)

    int pm_last_cx = -1;
    int pm_last_cy = -1;
};

} // namespace mve

#endif // MVE_TERRAIN_STREAMER_H
