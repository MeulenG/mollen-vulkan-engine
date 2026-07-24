#version 450

// Terrain vertex shader. Matches TerrainVertex in engine/scene/terrain_mesh.h:
//
//   location 0  vec3 position    - world-space, already WowToEngine-mapped
//   location 1  vec3 normal      - world-space, MCNR-derived (or fallback)
//   location 2  vec2 chunk_uv    - [0..1] within the chunk (samples alphas,
//                                  scaled up by kDiffuseRepeat in the
//                                  fragment shader to tile the diffuse).
//   location 3  uint chunk_index - linear MCNK index (0..255). flat-out
//                                  so the fragment shader can index the
//                                  per-chunk SSBO and alpha-array slice.
//   location 4  vec3 mccv        - per-vertex tint from MCNK's MCCV
//                                  sub-chunk. White when the chunk has
//                                  no painted tints. Multiplied into
//                                  the albedo in the fragment shader.

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_chunk_uv;
layout(location = 3) in uint in_chunk_index;
layout(location = 4) in vec3 in_mccv;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 model;
} push;

layout(location = 0) out vec3 frag_world_pos;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec2 frag_chunk_uv;
layout(location = 3) flat out uint frag_chunk_index;
layout(location = 4) out vec3 frag_mccv;

void main() {
    vec4 wp = push.model * vec4(in_position, 1.0);
    gl_Position = push.mvp * vec4(in_position, 1.0);
    frag_world_pos   = wp.xyz;
    frag_normal      = mat3(push.model) * in_normal;
    frag_chunk_uv    = in_chunk_uv;
    frag_chunk_index = in_chunk_index;
    frag_mccv        = in_mccv;
}
