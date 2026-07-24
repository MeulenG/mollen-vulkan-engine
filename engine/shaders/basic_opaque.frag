#version 450

// Opaque variant of basic.frag for M2 submeshes with blend_mode == 0
// (Opaque). Identical to basic.frag in lighting + fog math, but
// SKIPS the alpha-test discard so trunks and other no-alpha
// submeshes don't accidentally lose pixels.
//
// In WoW, M2 trunk submeshes use blend_mode 0 (Opaque) with BC1-RGB
// (no alpha) textures. Forcing those through the alpha-keyed shader
// has worked so far because BC1-RGB samples return alpha=1 (the
// post-swizzle default for missing components), which passes the
// 128/255 threshold trivially. But ANY future opaque submesh whose
// texture happens to have alpha < threshold (e.g. doodads that
// reuse a texture from a different material slot) would have its
// pixels punched out. Splitting opaque/alpha-key into separate
// shader paths is the architecturally correct fix.
//
// See basic.frag for the math derivation comments. This shader is
// kept in lockstep with basic.frag minus the discard line.

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

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

void main() {
    vec4 tex = texture(tex_sampler, frag_uv);
    // No discard - this pipeline is bound only for blend_mode == 0
    // Opaque submeshes (trunks, rocks, fences, building parts).

    // Two-sided lighting: flip N for back faces. Trunks are usually
    // single-sided (cylinders) but rendering both sides costs us
    // nothing extra here since the cullMode = eNone we set globally
    // already rasterizes both, and flipping N is the cheapest way
    // to keep back-face lighting sensible if we ever encounter a
    // double-sided opaque mesh.
    vec3 N = normalize(frag_normal);
    if (!gl_FrontFacing) N = -N;
    vec3 L = normalize(-scene.light_dir);
    float NdotL = max(dot(N, L), 0.0);

    // For opaque trunks we use straight Lambert (NO half-Lambert
    // wrap) because trunks aren't translucent and the wrap would
    // over-brighten the shadow side, giving them a faded / sun-
    // through-leaves appearance that's wrong on solid bark.
    vec3 lighting = scene.ambient_color + NdotL * scene.light_intensity * scene.direct_color;
    vec3 result = tex.rgb * frag_color * lighting;

    // WoW linear-ramp-with-pow fog. See basic.frag for derivation.
    float dist = length(frag_world_pos - scene.camera_pos);
    float span = max(1.0, scene.fog_end - scene.fog_start);
    float f1 = (scene.fog_end - dist) / span;
    float f2 = pow(clamp(f1, 0.0, 1.0), scene.fog_rate);
    float fog = 1.0 - f2;
    result = mix(result, scene.fog_color, fog);

    out_color = vec4(result, 1.0);
}
