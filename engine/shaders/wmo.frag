#version 450

// WMO fragment shader, v1 untextured. Uses MOCV1 vertex color as the
// fragment albedo (this is WoW's baked vertex lighting), modulated by
// a simple ambient + sun term and faded into fog at distance. Texture
// sampling lands in the next commit; this step gets the geometry
// visible with correct shading so the silhouettes of Northshire
// Abbey, Goldshire Inn etc. can be validated before the materials
// system grows in complexity.

layout(location = 0) in vec3 frag_world_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_uv1;
layout(location = 3) in vec2 frag_uv2;
layout(location = 4) in vec4 frag_color1;
layout(location = 5) in vec4 frag_color2;

layout(push_constant) uniform Push {
    mat4 mvp;
    mat4 model;
    vec4 sun_dir;          // xyz = direction TO surface (negate for L)
    vec4 sun_color;        // rgb
    vec4 ambient_color;    // rgb
    vec4 fog_color;        // rgb = tint, w = fog_end yards
    vec4 fog_params;       // x = fog_start, y = fog_rate, z = sidn_ramp
    vec4 camera_pos;
} push;

layout(location = 0) out vec4 out_color;

void main() {
    // MOCV1 alpha holds the interior/exterior blend factor (after the
    // loader's runtime fixup: 0 = pure interior, 1 = pure exterior).
    // Interior batches use MOCV1 RGB as the entire light term (the
    // artist baked the lighting). Exterior batches add a simple
    // sun + ambient on top.
    vec3 N = normalize(frag_normal);
    vec3 L = -normalize(push.sun_dir.xyz);
    float NdotL = max(dot(N, L), 0.0);

    // Half-Lambert wrap so unlit sides aren't black (same convention
    // as the M2 shader).
    float wrap_lambert = NdotL * 0.5 + 0.5;
    vec3 exterior_light = push.ambient_color.rgb + push.sun_color.rgb * wrap_lambert;

    // Interior light: pure ambient + MOCV1 vertex tint, which already
    // bakes in the per-vertex artist-painted lighting. The 2x scale
    // matches wowdev.wiki/WMO/Rendering note that MOCV is stored
    // half-range.
    vec3 mocv = frag_color1.rgb * 2.0;
    vec3 interior_light = push.ambient_color.rgb * 0.5 + mocv;

    // Lerp by the int/ext alpha. mocv is also applied to the exterior
    // side (subtle baked AO under overhangs etc.) by multiplying.
    float ext_blend = frag_color1.a;
    vec3 lit = mix(interior_light, exterior_light * mocv, ext_blend);

    // Albedo: until textures land, use the MOCV1 color directly so
    // walls/floors/roofs are visually distinguishable. This produces
    // a stylized "vertex-colored" look reminiscent of pre-texture
    // game-jam builds; textures replace this in the next commit.
    vec3 albedo = frag_color1.rgb + vec3(0.5);
    vec3 color = albedo * lit;

    // Linear-pow distance fog, same shape as terrain.frag.
    float dist = length(frag_world_pos - push.camera_pos.xyz);
    float fog_start = push.fog_params.x;
    float fog_end   = push.fog_color.w;
    float fog_rate  = push.fog_params.y;
    float f1 = clamp((fog_end - dist) / max(fog_end - fog_start, 0.001), 0.0, 1.0);
    float fog_blend = 1.0 - pow(f1, fog_rate);
    color = mix(color, push.fog_color.rgb, fog_blend);

    out_color = vec4(color, 1.0);
}
