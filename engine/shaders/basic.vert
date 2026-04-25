#version 450

// See docs/wiki/Math-Skeletal-Animation.md (GPU skinning)
// See docs/wiki/Math-Transforms.md (MVP)

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec2 in_uv;
layout(location = 4) in uvec4 in_bone_indices;
layout(location = 5) in vec4 in_bone_weights;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    mat4 model;
} push;

layout(set = 0, binding = 2) readonly buffer BoneBuffer {
    mat4 bone_matrices[];
} bones;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_world_pos;
layout(location = 3) out vec2 frag_uv;

void main() {
    mat4 skin_matrix =
        in_bone_weights.x * bones.bone_matrices[in_bone_indices.x] +
        in_bone_weights.y * bones.bone_matrices[in_bone_indices.y] +
        in_bone_weights.z * bones.bone_matrices[in_bone_indices.z] +
        in_bone_weights.w * bones.bone_matrices[in_bone_indices.w];

    vec4 skinned_pos = skin_matrix * vec4(in_position, 1.0);
    vec3 skinned_normal = mat3(skin_matrix) * in_normal;

    gl_Position = push.mvp * skinned_pos;
    frag_color = in_color;
    frag_normal = mat3(push.model) * skinned_normal;
    frag_world_pos = vec3(push.model * skinned_pos);
    frag_uv = in_uv;
}
