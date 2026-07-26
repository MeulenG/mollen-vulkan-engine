#version 450

// Foliage / alpha-key fragment shader (M2 blend_mode == 1).
//
// Why this exists separately from basic.frag:
//
// Foliage M2s are authored with "spherical normals" - leaf-card vertex
// normals point radially outward from the cluster center, not along
// the card's geometric normal. That's how WoW avoids the "cardboard
// cross" seam where two intersecting leaf planes meet (see
// m2_loader.cpp:372-427 for the rewrite). The cost: at the silhouette
// of every leaf cluster, the radial normal is perpendicular to the
// sun direction, so NdotL ~ 0 - giving a dim, desaturated "gray
// border" around every visible leaf shape. basic.frag's Lambert+wrap
// math hits ~0.075 lit factor at those silhouettes (vs ~0.71 at the
// center), a 10x brightness drop that paints leaf edges in pure
// ambient blue-gray.
//
// Fix: drop directional lighting entirely on alpha-key submeshes. WoW
// renders foliage flat (uniform brightness, no NdotL falloff) and we
// match that convention here. Result: leaves keep their painted
// color from edge to edge, no halo, and we sidestep the fact that the
// spherical-normal trick fundamentally breaks Lambert lighting at
// silhouettes.

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;     // unused, kept for vertex/frag interface match
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

// std140 layout matches engine::SceneUBO. See basic.frag for field
// semantics.
layout(set = 0, binding = 0) uniform SceneUBO {
    vec3 light_dir;
    float light_intensity;
    vec3 direct_color;
    float fog_rate;
    vec3 ambient_color;
    float fog_start;
    vec3 fog_color;
    float fog_end;
    vec3 camera_pos;
    float pad;
} scene;

layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

vec3 ACESToneMap(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    // Same 128/255 cutoff as basic.frag - WoW's canonical threshold.
    // Drives the silhouette; everything below it is invisible.
    vec4 tex = texture(tex_sampler, frag_uv);
    if (tex.a < 0.501960814) discard;

    // Flat lighting: ambient + full direct contribution, no NdotL.
    // The ambient is taken at the same 0.55 weight as basic.frag so
    // foliage doesn't look brighter than other geometry in shadow.
    // The "lit factor" is implicit 1.0 - every leaf reads as if it
    // were directly facing the sun.
    vec3 lighting = scene.ambient_color * 0.55 +
                    scene.light_intensity * scene.direct_color;
    vec3 result = tex.rgb * frag_color * lighting;

    // WoW client fog (linear ramp with pow shaping). Identical to
    // basic.frag - see that file for the derivation.
    float dist = length(frag_world_pos - scene.camera_pos);
    float span = max(1.0, scene.fog_end - scene.fog_start);
    float f1 = (scene.fog_end - dist) / span;
    float f2 = pow(clamp(f1, 0.0, 1.0), scene.fog_rate);
    float fog = 1.0 - f2;
    result = mix(result, scene.fog_color, fog);

    out_color = vec4(ACESToneMap(result), 1.0);
}
