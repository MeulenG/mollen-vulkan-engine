#version 450

// WMO group geometry. Vertex format matches WmoVertex:
//   location 0  vec3 position    - WMO-local frame
//   location 1  vec3 normal      - WMO-local frame
//   location 2  vec2 uv1         - texture coord layer 1
//   location 3  vec2 uv2         - texture coord layer 2 (TVerts2)
//   location 4  vec4 color1      - MOCV1 baked light + int/ext alpha
//   location 5  vec4 color2      - MOCV2 blend mask

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv1;
layout(location = 3) in vec2 in_uv2;
layout(location = 4) in vec4 in_color1;
layout(location = 5) in vec4 in_color2;

layout(push_constant) uniform Push {
    mat4 mvp;
    mat4 model;
    vec4 sun_dir;
    vec4 sun_color;
    vec4 ambient_color;
    vec4 fog_color;     // rgb = tint, w = fog_end yards
    vec4 fog_params;    // x = fog_start, y = fog_rate, z = sidn_ramp, w = unused
    vec4 camera_pos;
} push;

layout(location = 0) out vec3 frag_world_pos;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec2 frag_uv1;
layout(location = 3) out vec2 frag_uv2;
layout(location = 4) out vec4 frag_color1;
layout(location = 5) out vec4 frag_color2;

void main() {
    vec4 wp = push.model * vec4(in_position, 1.0);
    gl_Position    = push.mvp * vec4(in_position, 1.0);
    frag_world_pos = wp.xyz;
    frag_normal    = mat3(push.model) * in_normal;
    frag_uv1       = in_uv1;
    frag_uv2       = in_uv2;
    frag_color1    = in_color1;
    frag_color2    = in_color2;
}
