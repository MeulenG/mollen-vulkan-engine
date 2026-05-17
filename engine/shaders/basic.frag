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
    // Alpha test for blend-mode-1 (AlphaKey) submeshes. The WoW client
    // uses exactly 128/255 = 0.501960814 as its discard threshold
    // (verbatim from WebWowViewerCpp commonM2Material.glsl). The
    // previous 0.5 cutoff was 0.4% looser, just enough to keep a
    // fringe of half-transparent texels around every leaf cluster
    // which is what produced the jagged silhouettes.
    //
    // The discard is unconditional here because we don't yet plumb
    // per-submesh blend_mode into the fragment stage. Opaque-mode
    // textures have alpha=1 everywhere so the test never fires; a
    // future per-blend-mode pipeline split will gate this on
    // blend_mode == 1.
    vec4 tex = texture(tex_sampler, frag_uv);
    if (tex.a < 0.501960814) discard;

    // Two-sided lighting. The M2 pipeline now runs with
    // cullMode = eNone (see render_system.cpp comment) so the back
    // face of every leaf plane is also rasterized. Without flipping
    // N for back faces, that back face's normal still points "out"
    // of the original quad winding (away from the camera), so
    // NdotL = 0 on the side facing the viewer when the original
    // quad was angled away from the sun. The visible result is a
    // hard light/dark seam right down the middle of every crossed-
    // plane canopy - the "cardboard cross" look.
    //
    // gl_FrontFacing is a GLSL built-in that's true when the
    // primitive's winding matches the configured frontFace
    // (eCounterClockwise here). When false, we're looking at the
    // back side, so flip the normal to mirror the front-side
    // lighting. This matches what WoW's M2 material flag 0x04
    // "two-sided" causes the client to do.
    vec3 N = normalize(frag_normal);
    if (!gl_FrontFacing) N = -N;
    vec3 L = normalize(-scene.light_dir);
    float NdotL = max(dot(N, L), 0.0);

    // Half-Lambert wrap for foliage softness. Pure Lambert produces
    // a hard light/dark split because NdotL clamps to 0 - anything
    // facing > 90 deg from the sun is identically dark (ambient
    // only). Real leaves are thin and translucent: light wraps
    // around the shadow side, scattering through the leaf. The
    // standard cheap approximation (Valve's Half-Life 2 character
    // shading, Unreal foliage default) is:
    //   wrap = NdotL * 0.5 + 0.5    // remaps [-1..1] to [0..1]
    //   wrap = wrap * wrap          // re-bias toward shadow
    // The square keeps a recognizable lit/shadow direction cue but
    // softens the terminator so canopy crossing planes don't show
    // as sharp banding.
    float wrap = NdotL * 0.5 + 0.5;
    wrap = wrap * wrap;
    vec3 lighting = scene.ambient_color + wrap * scene.light_intensity * scene.direct_color;
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
