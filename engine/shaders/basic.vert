#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 model;
} push;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_world_pos;

void main() {
    gl_Position = push.mvp * vec4(in_position, 1.0);
    frag_color = in_color;
    frag_normal = mat3(push.model) * in_normal;
    frag_world_pos = vec3(push.model * vec4(in_position, 1.0));
}
