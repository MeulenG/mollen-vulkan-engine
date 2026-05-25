#version 450

// WMO fragment shader. Diffuse-only path for v1 - covers ~95% of
// Elwynn materials (Northshire stone, Goldshire wood, Stormwind
// brick). TwoLayerDiffuse / DiffuseEmissive / Env shaders fall back
// to plain diffuse for now; they look matte but the silhouette is
// correct.

layout(location = 0) in vec3 frag_world_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_uv1;
layout(location = 3) in vec2 frag_uv2;
layout(location = 4) in vec4 frag_color1;
layout(location = 5) in vec4 frag_color2;

// SceneUBO layout matches engine/systems/render_system.h SceneUBO.
// The std140 padding ordering MUST stay in lockstep.
layout(set = 0, binding = 0) uniform SceneUBO {
    vec3  light_dir;
    float light_intensity;
    vec3  direct_color;
    float fog_rate;
    vec3  ambient_color;
    float fog_start;
    vec3  fog_color;
    float fog_end;
    vec3  camera_pos;
    float pad;
} scene;

layout(set = 0, binding = 1) uniform sampler2D u_diffuse;

layout(location = 0) out vec4 out_color;

void main() {
    vec4 tex = texture(u_diffuse, frag_uv1);

    // MOCV1 alpha holds the interior/exterior blend factor (after the
    // WMO loader's runtime fixup: 0 = pure interior, 1 = pure exterior).
    vec3 N = normalize(frag_normal);
    vec3 L = -normalize(scene.light_dir);
    float NdotL = max(dot(N, L), 0.0);

    // Half-Lambert wrap so unlit sides aren't fully black.
    float wrap_lambert = NdotL * 0.5 + 0.5;
    vec3 exterior_light = scene.ambient_color +
                          scene.direct_color * wrap_lambert;

    // Interior: 2x MOCV1 baked lighting + reduced ambient. The 2x
    // matches wowdev.wiki/WMO/Rendering note that MOCV is stored
    // half-range.
    vec3 mocv = frag_color1.rgb * 2.0;
    vec3 interior_light = scene.ambient_color * 0.5 + mocv;

    float ext_blend = frag_color1.a;
    vec3 lit = mix(interior_light, exterior_light * mocv, ext_blend);

    vec3 color = tex.rgb * lit;

    // Linear-pow distance fog, same shape as terrain.frag.
    float dist = length(frag_world_pos - scene.camera_pos);
    float f1 = clamp((scene.fog_end - dist) /
                     max(scene.fog_end - scene.fog_start, 0.001),
                     0.0, 1.0);
    float fog_blend = 1.0 - pow(f1, scene.fog_rate);
    color = mix(color, scene.fog_color, fog_blend);

    out_color = vec4(color, tex.a);
}
