#version 450

// Water surface vertex shader. Matches WaterVertex in
// engine/scene/water_mesh.h:
//   location 0  vec3 position  - engine-space world coords
//   location 1  vec2 uv        - planar UV from world XZ * 0.06
//   location 2  float depth    - [0,1] for shallow/deep alpha blend

layout(location = 0) in vec3  in_position;
layout(location = 1) in vec2  in_uv;
layout(location = 2) in float in_depth;

layout(push_constant) uniform Push {
    mat4 mvp;
} push;

layout(location = 0) out vec2  frag_uv;
layout(location = 1) out float frag_depth;
layout(location = 2) out vec3  frag_world_pos;

void main() {
    gl_Position    = push.mvp * vec4(in_position, 1.0);
    frag_uv        = in_uv;
    frag_depth     = in_depth;
    frag_world_pos = in_position;
}
