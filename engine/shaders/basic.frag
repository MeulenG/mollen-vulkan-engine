#version 450

// See docs/wiki/Math-Lighting.md

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

// std140 layout matches engine::SceneUBO. Field naming follows the
// WoW Light.dbc / LightIntBand convention:
//   direct_color  = sun tint multiplied by Lambert dot(N, L)
//   ambient_color = vec3 ambient (cool blue-grey at Elwynn noon)
//   fog_rate      = pow exponent on the linear fog ramp; bends the
//                   ramp toward fog_end. 2.875 for Elwynn.
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

    // WoW client lighting model (M2 vertex shader, reverse-engineered
    // from wowserhq Wrath-Shading): pure Lambert + colored ambient,
    // no hemispherical / no specular. lightDir is stored "from sun
    // to world" (i.e. points down for an overhead sun), so we negate
    // to get L (the direction TO the sun) for the dot.
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(-scene.light_dir);
    float NdotL = max(dot(N, L), 0.0);
    vec3 lighting = scene.ambient_color + NdotL * scene.light_intensity * scene.direct_color;
    vec3 result = tex.rgb * frag_color * lighting;

    // WoW client fog (CShaderEffect::SetFogParams + vertex shader):
    //   f1 = (fog_end - dist) / (fog_end - fog_start)   in [0..1]
    //   f2 = pow(clamp(f1, 0, 1), fog_rate)
    //   fog = 1 - f2
    // This is a linear ramp with a pow shaping exponent, NOT the
    // exp/exp^2 used by classic D3DFOG_EXP/_EXP2. The pow bends the
    // curve so most of the fog accumulates near fog_end, which
    // matches how distant geometry in WoW transitions sharply to
    // haze instead of gradually fading from the camera.
    //
    // For Elwynn noon: fog_start=125, fog_end=500, fog_rate=2.875.
    // At dist=312 (midpoint): f1=0.5, f2=0.5^2.875=0.137, fog=0.863.
    // That gives ~86% fog at the geometric middle, not 50%.
    float dist = length(frag_world_pos - scene.camera_pos);
    float span = max(1.0, scene.fog_end - scene.fog_start);
    float f1 = (scene.fog_end - dist) / span;
    float f2 = pow(clamp(f1, 0.0, 1.0), scene.fog_rate);
    float fog = 1.0 - f2;
    result = mix(result, scene.fog_color, fog);

    out_color = vec4(result, 1.0);
}
