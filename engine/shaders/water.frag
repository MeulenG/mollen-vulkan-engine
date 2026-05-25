#version 450

// WoW water surface (Tier 1 / Tier 2 v1). Renders MH2O liquid patches
// with:
//   - Scrolling UV noise (procedural fbm) approximating river.N.blp
//     animation frames. T2 will replace this with the actual 30-frame
//     BLP texture array once the BLP loader supports animated sequences.
//   - Base color sampled from LightSnapshot.river_close (LightIntBand
//     row 15), pushed via the river_color uniform. Day/night driven.
//   - Depth-fade alpha: per-vertex depth attribute lerps between
//     shallow_alpha and deep_alpha, both from LightParams.dbc.
//   - WoW canonical "color + texture" additive composite (not multiply)
//     - this is what gives WoW water its characteristic bright look.
//   - Distance fog applied with the same linear-pow ramp as terrain so
//     the water at the horizon matches the sky-cone seam.

layout(location = 0) in vec2  frag_uv;
layout(location = 1) in float frag_depth;
layout(location = 2) in vec3  frag_world_pos;

layout(push_constant) uniform Push {
    // vertex stage block (offset 0..63)
    mat4 mvp;
    // fragment stage block (offset 64..111)
    // river_color.rgb = LightSnapshot.river_close (day/night driven)
    vec4 river_color;
    // fog_color.rgb = scene fog tint, fog_color.w = fog_end (yards)
    vec4 fog_color;
    // params.x = shallow_alpha  (LightParams.water_shallow_alpha)
    // params.y = deep_alpha     (LightParams.water_deep_alpha)
    // params.z = game time seconds (monotonic, drives scroll)
    // params.w = fog_start (yards)
    vec4 params;
    // camera.xyz = world cam pos (for distance-based fog)
    vec4 camera;
} push;

layout(location = 0) out vec4 out_color;

// IQ-style 2D value noise (same as background.frag - keeps the
// "WoW look" consistent between sky and water highlights).
float hash2(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float vnoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    float a = hash2(i + vec2(0, 0));
    float b = hash2(i + vec2(1, 0));
    float c = hash2(i + vec2(0, 1));
    float d = hash2(i + vec2(1, 1));
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(0.80, 0.60, -0.60, 0.80);
    for (int i = 0; i < 4; ++i) {
        v += a * vnoise(p);
        p = rot * p * 2.0;
        a *= 0.5;
    }
    return v;
}

void main() {
    float t = push.params.z;

    // Two layers of scrolling noise at different scales and speeds.
    // Layer 1: slow base ripple. Layer 2: faster smaller crests.
    // Hand-tuned to roughly match river.N.blp's perceived motion when
    // the BLPs animate at 60ms per frame.
    vec2 uv1 = frag_uv + vec2( 0.020,  0.013) * t;
    vec2 uv2 = frag_uv * 2.3 + vec2(-0.014,  0.025) * t;
    float n1 = fbm(uv1);
    float n2 = fbm(uv2);

    // Brightness of "specular noise" - the bright crests that look like
    // sun glints on the water surface. Powered up so only the peaks
    // show, giving the river its characteristic spangled look.
    float spangle = pow(clamp(n1 * 0.55 + n2 * 0.45, 0.0, 1.0), 2.0);

    // WoW's canonical water composite: base_color + texture_grey.
    // Additive (not multiplicative) - this is what produces the bright
    // river tone where Color1 is a deep aqua and the texture is
    // medium-grey caustic noise.
    vec3 base = push.river_color.rgb;
    vec3 highlight = vec3(spangle) * 0.45;
    vec3 diffuse = base + highlight;

    // Depth-fade alpha. shallow_alpha at depth=0 (waterline), deep_alpha
    // at depth=1 (river center). For Elwynn rivers this is typically
    // mix(0.45, 0.85, depth), giving translucent banks + opaque center.
    float alpha = mix(push.params.x, push.params.y, frag_depth);

    // Linear-pow distance fog, same shape as terrain.frag / basic.frag.
    // Without this, water at the horizon would punch through the
    // fog-tinted sky cone and produce a hard color seam.
    float dist = length(frag_world_pos - push.camera.xyz);
    float fog_start = push.params.w;
    float fog_end   = push.fog_color.w;
    float fog_rate  = 2.875;   // matches the SceneUBO fog rate at Elwynn noon
    float f1 = clamp((fog_end - dist) / max(fog_end - fog_start, 0.001),
                     0.0, 1.0);
    float fog_blend = 1.0 - pow(f1, fog_rate);
    diffuse = mix(diffuse, push.fog_color.rgb, fog_blend);

    out_color = vec4(diffuse, alpha);
}
