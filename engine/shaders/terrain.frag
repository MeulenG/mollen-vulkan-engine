#version 450

// Terrain fragment shader. Splat-blends up to four diffuse layers per
// MCNK chunk using the chunk's three alpha-map weights.
//
// Inputs from terrain.vert:
//   frag_world_pos    world position (used only for future fog/atmosphere)
//   frag_normal       interpolated MCNR normal (or fallback)
//   frag_chunk_uv     [0..1] within the chunk
//   frag_chunk_index  linear MCNK index (0..255), flat
//
// Math:
//   Each chunk's three upper-layer alpha maps are packed into the R/G/B
//   channels of one 64x64 slice of an alpha-map array texture. Sampling
//   at (frag_chunk_uv, frag_chunk_index) gives (w1, w2, w3) in [0..1].
//   Layer 0 is the base ("always covers"); its weight is
//     w0 = max(0, 1 - (w1 + w2 + w3))
//   so the four weights sum to exactly 1.0 when (w1 + w2 + w3) <= 1.
//
// The diffuse textures live in a separate 2D-array image whose slices
// were filled from BLPs at load time. The per-chunk SSBO stores which
// diffuse slice each of the four layers maps to (0xFFFFFFFF = unused).
// chunk_uv tiles by kDiffuseRepeat (=4) so the textures repeat at WoW's
// usual ~8.3 yard rate across the 33.3 yard chunk.

layout(location = 0) in vec3 frag_world_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_chunk_uv;
layout(location = 3) flat in uint frag_chunk_index;
layout(location = 4) in vec3 frag_mccv;

// std140 layout matches engine::SceneUBO. See basic.frag for the
// WoW Light.dbc field semantics.
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

// Per-chunk layer-slot lookup. Each chunk has 4 uints, one per layer,
// each pointing at a slice of the diffuse_array (or 0xFFFFFFFF =
// "skip this layer").
struct ChunkMeta { uvec4 layer_slot; };
layout(set = 0, binding = 1) readonly buffer ChunkMetaBuf {
    ChunkMeta chunks[];
} chunk_meta;

// Diffuse textures, one slice per unique BLP in the tile (typically
// 10-30 slices). All slices share the same width/height/format.
layout(set = 0, binding = 2) uniform sampler2DArray diffuse_array;

// Alpha maps, 256 slices (one per MCNK chunk). R = layer 1 weight,
// G = layer 2, B = layer 3, A reserved.
layout(set = 0, binding = 3) uniform sampler2DArray alpha_array;

layout(location = 0) out vec4 out_color;

// How many times the diffuse texture tiles across one chunk.
// 33.3 yards / 4 ~= 8.3 yards per repeat, which roughly matches the
// WoW client. Comment out the multiplier if the texture appears too
// large or too small for a given tile.
const float kDiffuseRepeat = 4.0;

// Sample a layer's diffuse, or return a fallback when the slot is
// unused. The fallback (magenta) is intentionally jarring so missing
// textures are easy to spot in editor.
vec3 SampleLayer(uint slot, vec2 uv) {
    if (slot == 0xFFFFFFFFu) {
        return vec3(0.0);
    }
    return texture(diffuse_array, vec3(uv, float(slot))).rgb;
}

void main() {
    uvec4 slots = chunk_meta.chunks[frag_chunk_index].layer_slot;

    // Layer 1/2/3 weights packed in R/G/B of the alpha slice.
    vec3 alpha = texture(alpha_array,
                         vec3(frag_chunk_uv, float(frag_chunk_index))).rgb;
    float w1 = alpha.r;
    float w2 = alpha.g;
    float w3 = alpha.b;
    float w0 = max(0.0, 1.0 - (w1 + w2 + w3));

    vec2 duv = frag_chunk_uv * kDiffuseRepeat;
    vec3 c0 = SampleLayer(slots.x, duv);
    vec3 c1 = SampleLayer(slots.y, duv);
    vec3 c2 = SampleLayer(slots.z, duv);
    vec3 c3 = SampleLayer(slots.w, duv);

    // If layer 0 is missing (no diffuse for the base), fall back to a
    // visible magenta only when all upper layers are also empty - that
    // way mostly-correct chunks don't get one bad sample stuck in the
    // mix.
    vec3 albedo;
    if (slots.x == 0xFFFFFFFFu &&
        slots.y == 0xFFFFFFFFu &&
        slots.z == 0xFFFFFFFFu &&
        slots.w == 0xFFFFFFFFu) {
        albedo = vec3(1.0, 0.0, 1.0);
    } else {
        albedo = c0 * w0 + c1 * w1 + c2 * w2 + c3 * w3;
    }

    // WoW lighting model: Lambert + colored ambient. See basic.frag.
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(-scene.light_dir);
    float NdotL = max(dot(N, L), 0.0);
    vec3 lighting = scene.ambient_color + NdotL * scene.light_intensity * scene.direct_color;

    // MCCV tint. WoW's "neutral" painted value is 0x7F / 255 ~ 0.5,
    // so the convention is: tint = 2 * MCCV. That makes 0.5 unchanged,
    // 1.0 a 2x overbright, 0.0 fully dark. The mesh builder defaults
    // unpainted vertices to 0.5 too, so they pass through neutral.
    vec3 tint = frag_mccv * 2.0;
    vec3 color = albedo * lighting * tint;

    // WoW linear-ramp-with-pow fog. See basic.frag for derivation.
    float dist = length(frag_world_pos - scene.camera_pos);
    float span = max(1.0, scene.fog_end - scene.fog_start);
    float f1 = (scene.fog_end - dist) / span;
    float f2 = pow(clamp(f1, 0.0, 1.0), scene.fog_rate);
    float fog = 1.0 - f2;
    color = mix(color, scene.fog_color, fog);

    out_color = vec4(color, 1.0);
}
