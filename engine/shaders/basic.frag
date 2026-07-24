#version 450

// See docs/wiki/Math-Lighting.md

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

// std140 layout matches engine::SceneUBO. Trailing fields are fog +
// camera position for the linear-distance fog calculation below.
layout(set = 0, binding = 0) uniform SceneUBO {
    vec3 light_dir;
    float ambient;
    vec3 light_color;
    float light_intensity;
    vec3 fog_color;
    float fog_start;
    vec3 camera_pos;
    float fog_end;
} scene;

layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main() {
    // Sample texture once - need the alpha channel for the cutout test
    // below. Most WoW prop M2s (trees, fences, bushes) ship with a
    // sub-mesh whose material is blend-mode 1 ("alpha-key"): the
    // texture has leaf/branch silhouettes painted onto a transparent
    // background, and the shader discards anything below ~50% alpha.
    // Without the discard, every plane renders fully filled and a
    // multi-plane tree mesh collapses into a solid pyramid shape.
    //
    // We don't yet have per-submesh blend modes plumbed through, so
    // this is unconditional. Opaque textures have alpha=1 everywhere
    // and the discard never fires; alpha-keyed textures get a clean
    // cutout. The 0.5 threshold matches the WoW client's convention.
    vec4 tex = texture(tex_sampler, frag_uv);
    if (tex.a < 0.5) discard;

    vec3 N = normalize(frag_normal);
    vec3 L = normalize(scene.light_dir);
    float diffuse = max(dot(N, L), 0.0);

    vec3 lighting = scene.ambient + diffuse * scene.light_intensity * scene.light_color;
    vec3 result = tex.rgb * frag_color * lighting;

    // Exponential squared fog, anchored to fog_start. Geometry closer
    // than fog_start is fully unfogged; the exp^2 ramp begins from
    // there. Without this anchor a pure 1-exp(-(d*k)^2) formulation
    // already adds ~15% haze at d=800 even when fog_end=2800, which
    // visibly bleaches near terrain.
    //
    // Math: ramp = max(0, d - fog_start). With density = 1.7 / (end -
    // start), at ramp=(end-start) we get fog = 1-exp(-1.7^2) ~= 0.94,
    // so geometry at fog_end is ~94% fog. 0..start is clear; start..
    // end is the haze gradient.
    float dist = length(frag_world_pos - scene.camera_pos);
    float ramp = max(0.0, dist - scene.fog_start);
    float span = max(1.0, scene.fog_end - scene.fog_start);
    float density = 1.7 / span;
    float fog = 1.0 - exp(-(ramp * density) * (ramp * density));
    fog = clamp(fog, 0.0, 1.0);
    result = mix(result, scene.fog_color, fog);

    out_color = vec4(result, 1.0);
}
