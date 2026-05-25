#include "light_cycle.h"

#include "dbc_file.h"
#include "schemas/wotlk/schema_light.h"
#include "schemas/wotlk/schema_light_params.h"
#include "schemas/wotlk/schema_light_int_band.h"
#include "schemas/wotlk/schema_light_float_band.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <exception>
#include <fstream>
#include <vector>

namespace mve {

namespace {

// Slurp a file into a byte buffer. Empty on failure - caller treats
// that as a soft fail and disables the cycle.
std::vector<uint8_t> ReadFileBytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0) return {};
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (!in.read(reinterpret_cast<char*>(buf.data()), size)) return {};
    return buf;
}

// Decode a BGRA-packed uint32 to a linear [0,1] RGB vec3.
// LightIntBand stores colors as BGRA where byte 0 = B, byte 1 = G,
// byte 2 = R. The /255 produces sRGB-encoded values; for our
// shader pipeline (which feeds these into a linear-light math chain)
// we keep them in sRGB-numerically-as-linear convention because the
// WoW client itself does that - it doesn't sRGB-decode these.
inline glm::vec3 BgraToRgb(uint32_t bgra) {
    float b = ((bgra >>  0) & 0xff) / 255.0f;
    float g = ((bgra >>  8) & 0xff) / 255.0f;
    float r = ((bgra >> 16) & 0xff) / 255.0f;
    return glm::vec3{r, g, b};
}

// Lerp two BGRA colors channel-wise (using BgraToRgb to decode).
inline glm::vec3 LerpBgra(uint32_t a, uint32_t b, float t) {
    glm::vec3 ca = BgraToRgb(a);
    glm::vec3 cb = BgraToRgb(b);
    return ca + (cb - ca) * t;
}

// Sample a sparse-keyframe int band at the given time (in WoW
// half-minute units [0, 2880)). Wraps around: the last keyframe
// connects back to the first across the 2880 boundary.
glm::vec3 SampleIntBand(const LightIntBandRow& row, float t_dbc) {
    if (row.count == 0) return glm::vec3{0.0f};
    if (row.count == 1) return BgraToRgb(row.bgra[0]);

    // Find the first keyframe with time > t_dbc. Linear scan is fine -
    // count is at most 16.
    uint32_t hi = 0;
    while (hi < row.count && row.time[hi] <= t_dbc) hi++;

    if (hi == 0) {
        // We're before the first keyframe; lerp from the last keyframe
        // (wrapped from the previous day) to the first.
        uint32_t t_last = row.time[row.count - 1];
        uint32_t t_first = row.time[0];
        // Distance from last keyframe to NOW, going forward through
        // the 2880 boundary.
        float span = static_cast<float>((2880u - t_last) + t_first);
        float pos  = static_cast<float>((2880u - t_last) + static_cast<uint32_t>(t_dbc));
        float f = (span > 0.0f) ? (pos / span) : 0.0f;
        return LerpBgra(row.bgra[row.count - 1], row.bgra[0], f);
    }
    if (hi >= row.count) {
        // We're past the last keyframe; lerp from last keyframe to
        // first (wrapping across the 2880 boundary into next day).
        uint32_t t_last = row.time[row.count - 1];
        uint32_t t_first = row.time[0];
        float span = static_cast<float>((2880u - t_last) + t_first);
        float pos  = t_dbc - static_cast<float>(t_last);
        float f = (span > 0.0f) ? (pos / span) : 0.0f;
        return LerpBgra(row.bgra[row.count - 1], row.bgra[0], f);
    }
    // Normal case: bracket [hi-1, hi].
    uint32_t lo = hi - 1;
    float tA = static_cast<float>(row.time[lo]);
    float tB = static_cast<float>(row.time[hi]);
    float f = (tB > tA) ? ((t_dbc - tA) / (tB - tA)) : 0.0f;
    return LerpBgra(row.bgra[lo], row.bgra[hi], f);
}

// Same wrap-around lerp for the float bands.
float SampleFloatBand(const LightFloatBandRow& row, float t_dbc) {
    if (row.count == 0) return 0.0f;
    if (row.count == 1) return row.value[0];

    uint32_t hi = 0;
    while (hi < row.count && row.time[hi] <= t_dbc) hi++;

    if (hi == 0) {
        uint32_t t_last = row.time[row.count - 1];
        uint32_t t_first = row.time[0];
        float span = static_cast<float>((2880u - t_last) + t_first);
        float pos  = static_cast<float>((2880u - t_last) + static_cast<uint32_t>(t_dbc));
        float f = (span > 0.0f) ? (pos / span) : 0.0f;
        return row.value[row.count - 1] + (row.value[0] - row.value[row.count - 1]) * f;
    }
    if (hi >= row.count) {
        uint32_t t_last = row.time[row.count - 1];
        uint32_t t_first = row.time[0];
        float span = static_cast<float>((2880u - t_last) + t_first);
        float pos  = t_dbc - static_cast<float>(t_last);
        float f = (span > 0.0f) ? (pos / span) : 0.0f;
        return row.value[row.count - 1] + (row.value[0] - row.value[row.count - 1]) * f;
    }
    uint32_t lo = hi - 1;
    float tA = static_cast<float>(row.time[lo]);
    float tB = static_cast<float>(row.time[hi]);
    float f = (tB > tA) ? ((t_dbc - tA) / (tB - tA)) : 0.0f;
    return row.value[lo] + (row.value[hi] - row.value[lo]) * f;
}

// Compute the sun direction from the day fraction t in [0, 1).
// 0 = midnight (sun below horizon, pointing up at the world from
// below), 0.25 = sunrise (low on the eastern horizon), 0.5 = noon
// (high in the south for our hemisphere convention), 0.75 = sunset
// on the west.
//
// We use a simple analytic sweep: the sun rotates 360 deg around the
// east-west axis with a fixed tilt. The result `light_dir` points
// FROM the sun TO the world (i.e. is the shadow direction); shaders
// negate to get L.
//
// This is a simplification of WoW's actual phi/theta tables which
// have slight wobble for seasonal variation - good enough for v1.
glm::vec3 SunDirection(float t_day) {
    // Solar azimuth: full 360 sweep over the day. Phase shift so
    // noon (t=0.5) puts the sun roughly overhead, slightly south.
    const float two_pi = 6.28318530718f;
    float angle = (t_day - 0.25f) * two_pi;   // noon -> angle = pi/2

    // Tilt of the sun's path from straight overhead. ~30 deg.
    const float tilt = 0.52f;  // ~30 deg in radians

    // Sun position: rotate around east-west axis (engine X), tilted
    // south (positive Z) by `tilt`.
    glm::vec3 sun_pos{
         std::cos(angle) * std::cos(tilt) * 0.0f - 0.0f,
         std::sin(angle),
         std::cos(angle) * std::sin(tilt)
    };
    // The above produces sun_pos.y = sin(angle):
    //   t=0.5 (noon)     angle =  pi/2  sin = +1  -> sun overhead
    //   t=0.25 (sunrise) angle =  0     sin =  0  -> horizon east
    //   t=0   (midnight) angle = -pi/2  sin = -1  -> sun under world
    //
    // light_dir points FROM sun TO world center = -sun_pos
    return -glm::normalize(sun_pos + glm::vec3{1e-4f, 0, 0});
}

} // namespace

bool LightTables::Load(const std::string& asset_dir) {
    pm_lights.clear();
    pm_params.clear();
    pm_int_bands.clear();
    pm_float_bands.clear();
    pm_loaded = false;

    // Helper: try to load and validate one DBC against its schema.
    //
    // CRITICAL: `bytes_out` MUST be owned by the caller's scope for at
    // least as long as `out` (the DbcFile) is read. DbcFile stores raw
    // pointers into the byte buffer, not a copy. If the buffer goes
    // out of scope while the DbcFile is still alive, reads return
    // dangling memory - which manifested as a silent hang on 2MB+
    // DBCs (LightIntBand) where the freed heap pages got reused by
    // later allocations.
    auto load_dbc = [&](const std::string& path,
                        const DbcSchema& schema,
                        std::vector<uint8_t>& bytes_out,
                        DbcFile& out) -> bool {
        bytes_out = ReadFileBytes(path);
        if (bytes_out.empty()) {
            std::fprintf(stderr, "LightTables: missing %s\n", path.c_str());
            return false;
        }
        if (!out.Load(bytes_out.data(),
                      static_cast<uint32_t>(bytes_out.size()))) {
            std::fprintf(stderr, "LightTables: parse failed %s\n", path.c_str());
            return false;
        }
        if (out.GetFieldCount() != schema.field_count ||
            out.GetRecordSize() != GetSchemaRecordSize(&schema)) {
            std::fprintf(stderr,
                "LightTables: schema mismatch for %s "
                "(fields %u vs %u, size %u vs %u)\n",
                path.c_str(),
                out.GetFieldCount(), schema.field_count,
                out.GetRecordSize(), GetSchemaRecordSize(&schema));
            return false;
        }
        return true;
    };

    // --- Light.dbc ---
    {
        DbcFile dbc;
        std::vector<uint8_t> bytes;
        if (!load_dbc(asset_dir + "/dbc/Light.dbc", schema_light, bytes, dbc))
            return false;
        pm_lights.reserve(dbc.GetRecordCount());
        for (uint32_t r = 0; r < dbc.GetRecordCount(); r++) {
            LightRow row{};
            row.id            = dbc.GetUInt32(r, 0);
            row.map_id        = dbc.GetUInt32(r, 1);
            row.position.x    = dbc.GetFloat(r, 2);
            row.position.y    = dbc.GetFloat(r, 3);
            row.position.z    = dbc.GetFloat(r, 4);
            row.falloff_start = dbc.GetFloat(r, 5);
            row.falloff_end   = dbc.GetFloat(r, 6);
            for (int i = 0; i < 8; i++) {
                row.light_params_ids[i] = dbc.GetUInt32(r, 7 + i);
            }
            pm_lights.push_back(row);
        }
    }

    // --- LightParams.dbc ---
    {
        DbcFile dbc;
        std::vector<uint8_t> bytes;
        if (!load_dbc(asset_dir + "/dbc/LightParams.dbc",
                      schema_light_params, bytes, dbc))
            return false;
        pm_params.reserve(dbc.GetRecordCount());
        for (uint32_t r = 0; r < dbc.GetRecordCount(); r++) {
            LightParamsRow row{};
            row.id                  = dbc.GetUInt32(r, 0);
            row.highlight_sky       = dbc.GetUInt32(r, 1);
            row.light_skybox_id     = dbc.GetUInt32(r, 2);
            row.cloud_type_id       = dbc.GetUInt32(r, 3);
            row.glow                = dbc.GetFloat(r, 4);
            row.water_shallow_alpha = dbc.GetFloat(r, 5);
            row.water_deep_alpha    = dbc.GetFloat(r, 6);
            row.ocean_shallow_alpha = dbc.GetFloat(r, 7);
            row.ocean_deep_alpha    = dbc.GetFloat(r, 8);
            pm_params.push_back(row);
        }
    }

    // --- LightIntBand.dbc ---
    {
        DbcFile dbc;
        std::vector<uint8_t> bytes;
        if (!load_dbc(asset_dir + "/dbc/LightIntBand.dbc",
                      schema_light_int_band, bytes, dbc))
            return false;
        uint32_t total = dbc.GetRecordCount();
        pm_int_bands.resize(total);
        for (uint32_t r = 0; r < total; r++) {
            LightIntBandRow& row = pm_int_bands[r];
            row.id    = dbc.GetUInt32(r, 0);
            row.count = std::min<uint32_t>(dbc.GetUInt32(r, 1), 16u);
            for (uint32_t i = 0; i < 16; i++) {
                row.time[i] = dbc.GetUInt32(r, 2 + i);
                row.bgra[i] = dbc.GetUInt32(r, 18 + i);
            }
        }
    }

    // --- LightFloatBand.dbc ---
    {
        DbcFile dbc;
        std::vector<uint8_t> bytes;
        if (!load_dbc(asset_dir + "/dbc/LightFloatBand.dbc",
                      schema_light_float_band, bytes, dbc))
            return false;
        pm_float_bands.reserve(dbc.GetRecordCount());
        for (uint32_t r = 0; r < dbc.GetRecordCount(); r++) {
            LightFloatBandRow row{};
            row.id    = dbc.GetUInt32(r, 0);
            row.count = std::min<uint32_t>(dbc.GetUInt32(r, 1), 16u);
            for (uint32_t i = 0; i < 16; i++) {
                row.time[i]  = dbc.GetUInt32(r, 2 + i);
                row.value[i] = dbc.GetFloat(r, 18 + i);
            }
            pm_float_bands.push_back(row);
        }
    }

    pm_loaded = true;
    std::fprintf(stderr,
        "LightTables: loaded %zu lights, %zu params, %zu int bands, %zu float bands\n",
        pm_lights.size(), pm_params.size(),
        pm_int_bands.size(), pm_float_bands.size());
    std::fflush(stderr);
    return true;
}

const LightRow* LightTables::GetLight(uint32_t id) const {
    for (const auto& r : pm_lights) {
        if (r.id == id) return &r;
    }
    return nullptr;
}

const LightParamsRow* LightTables::GetParams(uint32_t id) const {
    for (const auto& r : pm_params) {
        if (r.id == id) return &r;
    }
    return nullptr;
}

const LightIntBandRow* LightTables::GetIntBand(uint32_t id) const {
    for (const auto& r : pm_int_bands) {
        if (r.id == id) return &r;
    }
    return nullptr;
}

const LightFloatBandRow* LightTables::GetFloatBand(uint32_t id) const {
    for (const auto& r : pm_float_bands) {
        if (r.id == id) return &r;
    }
    return nullptr;
}

// -----------------------------------------------------------------------------
// LightCycle
// -----------------------------------------------------------------------------

void LightCycle::Tick(float dt_seconds) {
    if (pm_paused) return;
    pm_t_seconds += static_cast<double>(dt_seconds) *
                    static_cast<double>(pm_day_speed);
    // Real-time accumulator is unaffected by day_speed, so shader
    // animations (cloud scroll, water UV scroll) keep their natural
    // pacing regardless of whether the day cycle is sped up for editing.
    pm_real_seconds += static_cast<double>(dt_seconds);
}

float LightCycle::GetDayFraction() const {
    double t = std::fmod(pm_t_seconds, 86400.0);
    if (t < 0) t += 86400.0;
    return static_cast<float>(t / 86400.0);
}

void LightCycle::SetDayFraction(float f) {
    f = std::fmod(f, 1.0f);
    if (f < 0.0f) f += 1.0f;
    pm_t_seconds = static_cast<double>(f) * 86400.0;
}

// Compose a LightSnapshot from a LightParams id by sampling its 18
// IntBand rows + 6 FloatBand rows at the given DBC time. Returns
// a default-constructed snapshot if the params id isn't found.
static LightSnapshot SampleParams(const LightTables& tables,
                                   uint32_t params_id,
                                   float t_dbc) {
    LightSnapshot snap{};
    if (params_id == 0) return snap;

    // IntBand base index = (params_id - 1) * 18 + 1 (since IDs are
    // 1-indexed). Equivalent expression: params_id * 18 - 17.
    uint32_t int_base = params_id * 18u - 17u;

    auto sample_int = [&](uint32_t row_offset) -> glm::vec3 {
        const auto* row = tables.GetIntBand(int_base + row_offset);
        if (!row) return glm::vec3{0.0f};
        return SampleIntBand(*row, t_dbc);
    };

    snap.direct_color           = sample_int(0);
    snap.ambient_color          = sample_int(1);
    snap.sky_top                = sample_int(2);
    snap.sky_middle             = sample_int(3);
    snap.sky_band1              = sample_int(4);
    snap.sky_band2              = sample_int(5);
    snap.sky_smog               = sample_int(6);
    snap.sky_fog                = sample_int(7);
    snap.sun_color              = sample_int(8);
    snap.cloud_sun_color        = sample_int(9);
    snap.cloud_emissive         = sample_int(10);
    snap.cloud_layer1_ambient   = sample_int(11);
    snap.cloud_layer2_ambient   = sample_int(12);
    snap.ocean_close            = sample_int(13);
    snap.ocean_far              = sample_int(14);
    snap.river_close            = sample_int(15);
    snap.river_far              = sample_int(16);
    snap.shadow_opacity         = sample_int(17);

    uint32_t float_base = params_id * 6u - 5u;
    auto sample_float = [&](uint32_t row_offset) -> float {
        const auto* row = tables.GetFloatBand(float_base + row_offset);
        if (!row) return 0.0f;
        return SampleFloatBand(*row, t_dbc);
    };
    // Fog distance is stored as DBC value / 36 = yards.
    snap.fog_distance         = sample_float(0) / 36.0f;
    snap.fog_multiplier       = sample_float(1);
    snap.celestial_glow_through = sample_float(2);
    snap.cloud_density        = sample_float(3);
    snap.float_unk4           = sample_float(4);
    snap.float_unk5           = sample_float(5);

    // LightParams scalars (constant across the day - no time interp).
    if (const auto* p = tables.GetParams(params_id)) {
        snap.water_shallow_alpha = p->water_shallow_alpha;
        snap.water_deep_alpha    = p->water_deep_alpha;
        snap.ocean_shallow_alpha = p->ocean_shallow_alpha;
        snap.ocean_deep_alpha    = p->ocean_deep_alpha;
    }

    return snap;
}

// Lerp two snapshots component-wise. f = 0 returns a, f = 1 returns b.
static LightSnapshot LerpSnapshot(const LightSnapshot& a,
                                   const LightSnapshot& b,
                                   float f) {
    LightSnapshot o{};
    auto lerp3 = [&](const glm::vec3& va, const glm::vec3& vb) {
        return va + (vb - va) * f;
    };
    auto lerp1 = [&](float va, float vb) {
        return va + (vb - va) * f;
    };
    o.direct_color           = lerp3(a.direct_color, b.direct_color);
    o.ambient_color          = lerp3(a.ambient_color, b.ambient_color);
    o.sky_top                = lerp3(a.sky_top, b.sky_top);
    o.sky_middle             = lerp3(a.sky_middle, b.sky_middle);
    o.sky_band1              = lerp3(a.sky_band1, b.sky_band1);
    o.sky_band2              = lerp3(a.sky_band2, b.sky_band2);
    o.sky_smog               = lerp3(a.sky_smog, b.sky_smog);
    o.sky_fog                = lerp3(a.sky_fog, b.sky_fog);
    o.sun_color              = lerp3(a.sun_color, b.sun_color);
    o.cloud_sun_color        = lerp3(a.cloud_sun_color, b.cloud_sun_color);
    o.cloud_emissive         = lerp3(a.cloud_emissive, b.cloud_emissive);
    o.cloud_layer1_ambient   = lerp3(a.cloud_layer1_ambient, b.cloud_layer1_ambient);
    o.cloud_layer2_ambient   = lerp3(a.cloud_layer2_ambient, b.cloud_layer2_ambient);
    o.ocean_close            = lerp3(a.ocean_close, b.ocean_close);
    o.ocean_far              = lerp3(a.ocean_far, b.ocean_far);
    o.river_close            = lerp3(a.river_close, b.river_close);
    o.river_far              = lerp3(a.river_far, b.river_far);
    o.shadow_opacity         = lerp3(a.shadow_opacity, b.shadow_opacity);
    o.fog_distance           = lerp1(a.fog_distance, b.fog_distance);
    o.fog_multiplier         = lerp1(a.fog_multiplier, b.fog_multiplier);
    o.celestial_glow_through = lerp1(a.celestial_glow_through, b.celestial_glow_through);
    o.cloud_density          = lerp1(a.cloud_density, b.cloud_density);
    o.float_unk4             = lerp1(a.float_unk4, b.float_unk4);
    o.float_unk5             = lerp1(a.float_unk5, b.float_unk5);
    o.water_shallow_alpha    = lerp1(a.water_shallow_alpha, b.water_shallow_alpha);
    o.water_deep_alpha       = lerp1(a.water_deep_alpha, b.water_deep_alpha);
    o.ocean_shallow_alpha    = lerp1(a.ocean_shallow_alpha, b.ocean_shallow_alpha);
    o.ocean_deep_alpha       = lerp1(a.ocean_deep_alpha, b.ocean_deep_alpha);
    o.light_dir              = a.light_dir;  // derived later, not lerped
    return o;
}

LightSnapshot LightCycle::Sample(const glm::vec3& cam_pos,
                                  uint32_t map_id) const {
    if (!pm_tables || !pm_tables->IsLoaded()) {
        LightSnapshot fallback{};
        fallback.light_dir = SunDirection(GetDayFraction());
        return fallback;
    }

    float t_day = GetDayFraction();
    float t_dbc = t_day * 2880.0f;  // half-minutes per WoW day

    // V1 spatial blend: find the closest spatial Light.dbc entry on
    // the requested map within its falloff_end, plus the worldwide
    // default (X=Y=Z=0) on the same map. Blend the two by the
    // closest entry's distance-weight.
    //
    // The worldwide default is conventionally Light.dbc ID=1 with
    // (0,0,0) position and very large falloff. We find it by scanning
    // for the entry on this map with position == 0.
    const LightRow* worldwide = nullptr;
    const LightRow* spatial = nullptr;
    float best_weight = 0.0f;

    for (const auto& row : pm_tables->AllLights()) {
        if (row.map_id != map_id) continue;
        bool is_origin = (row.position.x == 0.0f &&
                          row.position.y == 0.0f &&
                          row.position.z == 0.0f);
        if (is_origin) {
            // Pick the first origin entry as the worldwide default.
            // There's typically only one per map.
            if (!worldwide) worldwide = &row;
            continue;
        }
        if (row.falloff_end <= 0.0f) continue;
        float dist = glm::length(cam_pos - row.position);
        if (dist >= row.falloff_end) continue;
        // Weight: 0 at falloff_end, 1 at falloff_start (or closer).
        float w = 1.0f;
        if (row.falloff_end > row.falloff_start) {
            w = 1.0f - (dist - row.falloff_start) /
                       (row.falloff_end - row.falloff_start);
            w = std::clamp(w, 0.0f, 1.0f);
        }
        if (w > best_weight) {
            best_weight = w;
            spatial = &row;
        }
    }

    // Weather state: v1 hardcodes "Clear" = light_params_ids[0].
    auto params_id_of = [](const LightRow* row) -> uint32_t {
        return row ? row->light_params_ids[0] : 0u;
    };

    LightSnapshot ws_snap = SampleParams(*pm_tables,
                                          params_id_of(worldwide), t_dbc);
    LightSnapshot result = ws_snap;

    if (spatial && best_weight > 0.0f) {
        LightSnapshot sp_snap = SampleParams(*pm_tables,
                                              params_id_of(spatial), t_dbc);
        result = LerpSnapshot(ws_snap, sp_snap, best_weight);
    }

    // Derived: sun direction from time.
    result.light_dir = SunDirection(t_day);

    return result;
}

} // namespace mve
