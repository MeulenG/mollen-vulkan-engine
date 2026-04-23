#version 450

layout(location = 0) in vec3 in_position;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 model;
} push;

layout(location = 0) out vec3 frag_world_pos;

void main() {
    vec4 world_pos = push.model * vec4(in_position, 1.0);
    gl_Position = push.mvp * vec4(in_position, 1.0);
    frag_world_pos = world_pos.xyz;
}
