#include "terrain_streamer.h"

#include "components/terrain_tile_component.h"
#include "../formats/adt_types.h"
#include "../formats/wdt_loader.h"

#include <algorithm>
#include <cstdio>

namespace mve {

namespace {

// Resolve a WoW-style backslashed asset path against MVE_ASSET_DIR.
// Mirrors the helper in asset_manager.cpp - kept local to avoid
// cross-translation-unit coupling.
std::string ResolveWowAsset(const std::string& wow_path) {
    std::string out = std::string(MVE_ASSET_DIR) + "/" + wow_path;
    std::replace(out.begin(), out.end(), '\\', '/');
    return out;
}

inline int Clamp63(int v) { return std::clamp(v, 0, 63); }

} // namespace

TerrainStreamer::TerrainStreamer(AssetManager& assets, Scene& scene)
    : pm_assets{assets}, pm_scene{scene} {
    pm_tile_exists.assign(64 * 64, 0);
}

bool TerrainStreamer::LoadWdt(const std::string& wow_wdt_path) {
    std::string fs_path = ResolveWowAsset(wow_wdt_path);
    std::vector<WdtTile> existing;
    if (!WdtLoader::LoadFile(fs_path, existing)) {
        std::fprintf(stderr, "WDT load failed: %s\n", fs_path.c_str());
        pm_wdt_loaded = false;
        return false;
    }
    pm_tile_exists.assign(64 * 64, 0);
    for (const auto& t : existing) {
        if (t.x >= 0 && t.x < 64 && t.y >= 0 && t.y < 64) {
            pm_tile_exists[t.y * 64 + t.x] = 1;
        }
    }
    pm_wdt_loaded = true;
    return true;
}

bool TerrainStreamer::TileExists(int tile_x, int tile_y) const {
    if (tile_x < 0 || tile_x > 63) return false;
    if (tile_y < 0 || tile_y > 63) return false;
    // If we never loaded a WDT, fall through to "always try" so the
    // streamer can still be exercised without it. Missing files are
    // handled gracefully in LoadAdtTileIntoScene.
    if (!pm_wdt_loaded) return true;
    return pm_tile_exists[tile_y * 64 + tile_x] != 0;
}

void TerrainStreamer::EngineToTile(const glm::vec3& engine_pos,
                                    int& tile_x, int& tile_y) const {
    // Engine.x = WoW Y (east). Engine.z = WoW X (south).
    // For any WoW coord w, the tile index is 32 - w / kAdtTileSize.
    // That's because tile 32 sits at the world's WoW origin and tile
    // indices grow toward NEGATIVE WoW coords (per the file format).
    float tx_f = 32.0f - engine_pos.z / kAdtTileSize;
    float ty_f = 32.0f - engine_pos.x / kAdtTileSize;
    tile_x = Clamp63(static_cast<int>(std::floor(tx_f)));
    tile_y = Clamp63(static_cast<int>(std::floor(ty_f)));
}

bool TerrainStreamer::TilePresent(int tile_x, int tile_y) const {
    bool present = false;
    pm_scene.Each<TerrainTileComponent>(
        [&](Entity&, TerrainTileComponent& t) {
            if (t.pm_tile_x == tile_x && t.pm_tile_y == tile_y) present = true;
        });
    return present;
}

void TerrainStreamer::PreloadAround(int cx, int cy, int r,
                                     const vk::raii::DescriptorSetLayout& terrain_layout) {
    // Synchronous load of all valid tiles in the square. One-time hit at
    // startup; ~150ms per tile so a 3x3 = ~1.3s freeze. Async streaming
    // is a future R3.5.
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (tx < 0 || tx > 63 || ty < 0 || ty > 63) continue;
            if (!TileExists(tx, ty)) continue;
            if (TilePresent(tx, ty)) continue;
            pm_assets.LoadAdtTileIntoScene(tx, ty, pm_scene, terrain_layout);
        }
    }
    pm_last_cx = cx;
    pm_last_cy = cy;
}

void TerrainStreamer::Update(const glm::vec3& camera_pos_engine,
                              const vk::raii::DescriptorSetLayout& terrain_layout) {
    int cx, cy;
    EngineToTile(camera_pos_engine, cx, cy);
    if (cx == pm_last_cx && cy == pm_last_cy) return;
    pm_last_cx = cx;
    pm_last_cy = cy;

    // 1. Load any missing tile inside the inner ring.
    for (int dy = -pm_radius; dy <= pm_radius; dy++) {
        for (int dx = -pm_radius; dx <= pm_radius; dx++) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (tx < 0 || tx > 63 || ty < 0 || ty > 63) continue;
            if (!TileExists(tx, ty)) continue;
            if (TilePresent(tx, ty)) continue;
            pm_assets.LoadAdtTileIntoScene(tx, ty, pm_scene, terrain_layout);
        }
    }

    // 2. Evict tiles outside the outer ring (hysteresis - we don't
    //    evict at the inner ring boundary because that thrashes when
    //    the camera wobbles on a tile edge).
    std::vector<uint32_t> to_destroy;
    pm_scene.Each<TerrainTileComponent>(
        [&](Entity& e, TerrainTileComponent& t) {
            int adx = std::abs(t.pm_tile_x - cx);
            int ady = std::abs(t.pm_tile_y - cy);
            if (adx > pm_evict_radius || ady > pm_evict_radius) {
                to_destroy.push_back(e.Id());
            }
        });

    // Before destroying entities, drain any in-flight command buffers
    // that might still be sampling the descriptor sets we're about to
    // free. AssetManager's pool was created with eFreeDescriptorSet so
    // the free call itself is legal; the constraint is that the GPU
    // must be done with the set first.
    //
    // waitIdle is a heavy hammer for what's typically a 1-tile crossing
    // event - if eviction becomes more frequent we'll want a real
    // per-frame retirement queue, but for v1 the latency is acceptable
    // and the alternative is the validation error the user just saw.
    if (!to_destroy.empty()) {
        pm_assets.GetDevice().GetDevice().waitIdle();
    }
    for (uint32_t id : to_destroy) {
        pm_scene.DestroyEntity(id);
    }
}

} // namespace mve
