#ifndef MVE_LIGHT_CYCLE_H
#define MVE_LIGHT_CYCLE_H

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace mve {

// One LightIntBand or LightFloatBand row, before any interpolation.
// Both share the same {timestamps[16], values[16], count} shape - we
// store them as separate templated structs anyway so the value type
// (BGRA-uint32 for int bands, float for float bands) stays explicit.
struct LightIntBandRow {
    uint32_t id = 0;
    uint32_t time[16] = {0};            // half-minutes 0..2880
    uint32_t bgra[16] = {0};            // BGRA-packed colors
    uint32_t count = 0;                 // sparse: only 'count' entries are valid
};

struct LightFloatBandRow {
    uint32_t id = 0;
    uint32_t time[16] = {0};            // half-minutes 0..2880
    float    value[16] = {0.0f};
    uint32_t count = 0;
};

// One LightParams.dbc row. Each Light.dbc entry has 8 of these (one
// per weather state). Used as the foreign key into the IntBand /
// FloatBand row groups via row-index arithmetic:
//
//   IntBand   base index = params_id * 18 - 17  (rows +0..+17)
//   FloatBand base index = params_id *  6 -  5  (rows +0..+5)
struct LightParamsRow {
    uint32_t id = 0;
    uint32_t highlight_sky = 0;         // bool flag
    uint32_t light_skybox_id = 0;       // -> LightSkybox.dbc
    uint32_t cloud_type_id = 0;
    float    glow = 0.0f;
    float    water_shallow_alpha = 0.0f;
    float    water_deep_alpha = 0.0f;
    float    ocean_shallow_alpha = 0.0f;
    float    ocean_deep_alpha = 0.0f;
};

// One Light.dbc row. Spatial entries per MapID; the client picks the
// closest entry within `falloff_end` and interpolates toward the
// worldwide-default entry (X=Y=Z=0, ID=1 on Map 0). Each row has 8
// LightParamsID columns for the weather states (Clear, ClearWater,
// Storm, StormWater, Death, Unk1, Unk2, Unk3).
struct LightRow {
    uint32_t id = 0;
    uint32_t map_id = 0;
    glm::vec3 position{0.0f};
    float falloff_start = 0.0f;
    float falloff_end = 0.0f;
    uint32_t light_params_ids[8] = {0};
};

// A fully-interpolated snapshot of all the light values at one
// (time, position, weather) point. This is what the runtime maps
// onto SceneUBO + the background-sky push constants per frame.
//
// Field names match LightIntBand row indices 0..17 (BGRA->RGB
// decoded and normalised to [0,1] vec3) plus the 6 FloatBand rows
// at the end. Sun direction is derived from a phi/theta table keyed
// off the day fraction.
struct LightSnapshot {
    // LightIntBand row 0..17, decoded to RGB [0,1]
    glm::vec3 direct_color{1.0f};        // 0 - sun tint
    glm::vec3 ambient_color{0.4f};       // 1 - hemispheric ambient
    glm::vec3 sky_top{0.0f};              // 2 - zenith
    glm::vec3 sky_middle{0.3f};           // 3 - sky body
    glm::vec3 sky_band1{0.7f};            // 4
    glm::vec3 sky_band2{0.8f};            // 5
    glm::vec3 sky_smog{0.7f};             // 6
    glm::vec3 sky_fog{0.3f};              // 7 - also drives world fog color
    glm::vec3 sun_color{0.3f};            // 8 - sun-disc / halo
    glm::vec3 cloud_sun_color{1.0f};      // 9
    glm::vec3 cloud_emissive{1.0f};       // 10
    glm::vec3 cloud_layer1_ambient{0.4f}; // 11
    glm::vec3 cloud_layer2_ambient{0.4f}; // 12
    glm::vec3 ocean_close{0.4f};          // 13
    glm::vec3 ocean_far{0.2f};            // 14
    glm::vec3 river_close{0.4f};          // 15
    glm::vec3 river_far{0.2f};            // 16
    glm::vec3 shadow_opacity{0.3f};       // 17 - used as alpha-shadow mult

    // LightFloatBand row 0..5
    float fog_distance = 500.0f;          // 0 - in yards (DBC * (1/36))
    float fog_multiplier = 0.25f;         // 1 - fog_start = fog_distance * mult
    float celestial_glow_through = 1.0f;  // 2 - sun/moon brightness through clouds
    float cloud_density = 0.5f;           // 3
    float float_unk4 = 0.95f;             // 4
    float float_unk5 = 1.0f;              // 5

    // Derived: not from the DBCs directly, but from a phi/theta sun
    // table keyed off `t_dbc / 2880`. World-space direction pointing
    // FROM the sun TO the world (i.e. shadow direction). Shaders
    // negate to get L.
    glm::vec3 light_dir{0.0f, -1.0f, 0.0f};
};

// In-memory lookup tables loaded once at startup from
//   assets/dbc/Light.dbc
//   assets/dbc/LightParams.dbc
//   assets/dbc/LightIntBand.dbc
//   assets/dbc/LightFloatBand.dbc
//
// Held by AssetManager (mirroring the GroundEffectTables pattern).
class LightTables {
public:
    bool Load(const std::string& asset_dir);

    const LightRow*           GetLight(uint32_t id) const;
    const LightParamsRow*     GetParams(uint32_t id) const;
    const LightIntBandRow*    GetIntBand(uint32_t id) const;
    const LightFloatBandRow*  GetFloatBand(uint32_t id) const;

    const std::vector<LightRow>& AllLights() const { return pm_lights; }

    size_t LightCount()      const { return pm_lights.size(); }
    size_t ParamsCount()     const { return pm_params.size(); }
    size_t IntBandCount()    const { return pm_int_bands.size(); }
    size_t FloatBandCount()  const { return pm_float_bands.size(); }

    bool IsLoaded() const { return pm_loaded; }

private:
    std::vector<LightRow>          pm_lights;
    std::vector<LightParamsRow>    pm_params;
    std::vector<LightIntBandRow>   pm_int_bands;
    std::vector<LightFloatBandRow> pm_float_bands;
    bool pm_loaded = false;
};

// The runtime day/night cycle. Owns a clock (configurable game-day
// length) and produces a fresh LightSnapshot per frame by:
//
//   1. Temporal interp: lerp each LightIntBand / LightFloatBand row
//      between its two bracketing keyframes at the current time.
//   2. Spatial blend: find Light.dbc entries near the camera, weight
//      by 1 - dist/falloff_end, blend toward the worldwide-default
//      entry (X=Y=Z=0, ID=1 on Map 0).
//   3. Weather state: hardcoded "Clear" (column 0 of light_params_ids)
//      for v1; future versions can blend toward Storm/Death.
//
// The output is fed into RenderSystem::pm_scene_data + the background
// sky-cone push constants each frame.
//
// Game time semantics: pm_t_seconds is a continuous accumulator. Mod
// 86400 to get the current second-in-day. game_day_seconds defaults
// to 1440 (= 24 game minutes = WoW's standard 60x acceleration).
class LightCycle {
public:
    void SetTables(const LightTables* tables) { pm_tables = tables; }

    // Advance the internal clock. Call once per frame with delta-time
    // in real seconds. Does nothing if pm_paused is true.
    void Tick(float dt_seconds);

    // Compute a LightSnapshot for the current time + camera position
    // on the given map. mapID=0 means Azeroth (Eastern Kingdoms +
    // Kalimdor for vanilla content).
    LightSnapshot Sample(const glm::vec3& cam_pos, uint32_t map_id) const;

    // Convenience: get/set the current time in [0, 1] day fraction.
    // 0 = midnight, 0.25 = 06:00, 0.5 = noon, 0.75 = 18:00.
    float GetDayFraction() const;
    void  SetDayFraction(float f);

    // Get/set the current time in hours [0, 24).
    float GetHour() const         { return GetDayFraction() * 24.0f; }
    void  SetHour(float h)        { SetDayFraction(h / 24.0f); }

    // Day speed multiplier. 1.0 = real-time (1 sec real = 1 sec game).
    // WoW default is 60.0 (24 game min per real-time day). The
    // editor's "real-time" toggle scrubs this to 60 for live preview.
    float GetDaySpeed() const     { return pm_day_speed; }
    void  SetDaySpeed(float s)    { pm_day_speed = s; }

    bool  IsPaused() const        { return pm_paused; }
    void  SetPaused(bool p)       { pm_paused = p; }

    // Continuous game-time accumulator in seconds. Monotonically
    // increasing (does NOT wrap at 86400) so it can drive shader
    // animations (cloud scroll, water UV scroll) without discontinuity
    // at the day boundary. Cast to float at the use site; precision
    // stays good for ~48 hours of continuous play.
    double GetGameTimeSeconds() const { return pm_t_seconds; }

private:
    const LightTables* pm_tables = nullptr;
    double pm_t_seconds = 12.0 * 3600.0;     // start at noon
    float  pm_day_speed = 60.0f;             // WoW default: 24 min per day
    bool   pm_paused = false;
};

} // namespace mve

#endif // MVE_LIGHT_CYCLE_H
