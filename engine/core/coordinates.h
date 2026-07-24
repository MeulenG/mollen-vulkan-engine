#ifndef MVE_COORDINATES_H
#define MVE_COORDINATES_H

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace mve::coords {

// Engine coordinate convention - adopted from Kelsidavis/WoWee
// (include/core/coordinates.hpp). Z-up right-handed. The full
// rationale + all four conventions involved are documented below.
//
// ---- Canonical WoW world coordinate system (per-map) ----
//   +X = North, +Y = West, +Z = Up (height)
//   Origin (0,0,0) is the center of the 64x64 ADT tile grid.
//   Full extent: +-17066.66656 in X and Y.
//
// ---- Engine rendering coordinate system (THIS engine) ----
//   renderX = wowY (west axis, positive = west)
//   renderY = wowX (north axis, positive = north)
//   renderZ = wowZ (up axis, positive = up)
//   Terrain vertices (MCNK) are stored directly in this space.
//   This is what every WMO / M2 / water mesh ultimately renders in.
//
// ---- ADT file placement coordinate system ----
//   Used by MDDF (doodads) and MODF (WMOs) records in ADT files.
//   Range [0, 34133.333] with center at ZEROPOINT (17066.666).
//   adtY = height; adtX/adtZ are horizontal.
//
// ---- Server / emulator coordinate system ----
//   WoW emulators (TC, MaNGOS) send positions over the wire as
//   (X, Y, Z) where server.X = canonical.Y (west axis), server.Y =
//   canonical.X (north axis), server.Z = canonical.Z (height). This
//   is also the byte order inside movement packets.

inline constexpr float TILE_SIZE = 533.33333f;
inline constexpr float ZEROPOINT = 32.0f * TILE_SIZE;     // 17066.66656
inline constexpr float PI = 3.14159265358979323846f;
inline constexpr float TWO_PI = 6.28318530717958647692f;

// Convert server/wire coordinates -> canonical WoW coordinates.
inline glm::vec3 ServerToCanonical(const glm::vec3& server) {
    return {server.y, server.x, server.z};
}

// Convert canonical WoW coordinates -> server/wire coordinates.
inline glm::vec3 CanonicalToServer(const glm::vec3& canonical) {
    return {canonical.y, canonical.x, canonical.z};
}

// Normalize angle to [-PI, PI].
inline float NormalizeAngleRad(float a) {
    while (a > PI) a -= TWO_PI;
    while (a < -PI) a += TWO_PI;
    return a;
}

// Convert server/wire yaw -> canonical yaw. Canonical convention:
//   North = 0, East = +pi/2, South = +-pi, West = -pi/2.
inline float ServerToCanonicalYaw(float server_yaw) {
    return NormalizeAngleRad(server_yaw - PI * 0.5f);
}

inline float CanonicalToServerYaw(float canonical_yaw) {
    return NormalizeAngleRad(canonical_yaw + PI * 0.5f);
}

// Canonical <-> render conversions are a single X<->Y swap.
inline glm::vec3 CanonicalToRender(const glm::vec3& wow) {
    return {wow.y, wow.x, wow.z};
}

inline glm::vec3 RenderToCanonical(const glm::vec3& render) {
    return {render.y, render.x, render.z};
}

// ADT file placement data (MDDF/MODF) -> engine rendering coords.
// Both horizontal axes mirror through ZEROPOINT; the height axis is
// just relabeled (adtY is already "height" on disk, here it lands in
// renderZ which is also "height").
inline glm::vec3 AdtToWorld(float adt_x, float adt_y, float adt_z) {
    return {
        -(adt_z - ZEROPOINT),    // renderX = ZP - adtZ  (= wowY, west)
        -(adt_x - ZEROPOINT),    // renderY = ZP - adtX  (= wowX, north)
        adt_y                     // renderZ = adtY       (= wowZ, up)
    };
}

inline glm::vec3 AdtToWorld(const glm::vec3& adt) {
    return AdtToWorld(adt.x, adt.y, adt.z);
}

// Engine rendering coordinates -> ADT file placement data.
inline glm::vec3 WorldToAdt(float render_x, float render_y, float render_z) {
    return {
        ZEROPOINT - render_y,    // adtX = ZP - renderY (= ZP - wowX)
        render_z,                 // adtY = renderZ      (= wowZ, height)
        ZEROPOINT - render_x      // adtZ = ZP - renderX (= ZP - wowY)
    };
}

inline glm::vec3 WorldToAdt(const glm::vec3& world) {
    return WorldToAdt(world.x, world.y, world.z);
}

// Engine render coords -> ADT tile indices. Returns (tile_x, tile_y)
// matching the filename pattern Map_<tile_x>_<tile_y>.adt where
// tile_x is the column (E-W, canonical.Y subdivision) and tile_y is
// the row (N-S, canonical.X subdivision). Engine render basis here is
// renderX=canonical.X (north), renderY=canonical.Y (west), so the
// column derives from renderY and the row from renderX.
inline std::pair<int, int> WorldToTile(float render_x, float render_y) {
    int tile_x = static_cast<int>(std::floor(32.0f - render_y / TILE_SIZE));
    int tile_y = static_cast<int>(std::floor(32.0f - render_x / TILE_SIZE));
    tile_x = std::clamp(tile_x, 0, 63);
    tile_y = std::clamp(tile_y, 0, 63);
    return {tile_x, tile_y};
}

// Canonical WoW coords -> ADT tile indices (column, row).
inline std::pair<int, int> CanonicalToTile(float wow_x, float wow_y) {
    int tile_x = static_cast<int>(std::floor(32.0f - wow_y / TILE_SIZE));
    int tile_y = static_cast<int>(std::floor(32.0f - wow_x / TILE_SIZE));
    tile_x = std::clamp(tile_x, 0, 63);
    tile_y = std::clamp(tile_y, 0, 63);
    return {tile_x, tile_y};
}

} // namespace mve::coords

#endif // MVE_COORDINATES_H
