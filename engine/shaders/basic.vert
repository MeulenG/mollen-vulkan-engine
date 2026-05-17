#version 450

// See docs/wiki/Math-Skeletal-Animation.md (GPU skinning)
// See docs/wiki/Math-Transforms.md (MVP)
//
// R4.5: instancing.
//
// All M2 entities (legacy spell-editor previews and instanced doodads)
// share this shader. To keep one pipeline that serves both, we always
// read the per-instance model matrix from an SSBO at binding 3.
//
//   gl_Position = push.mvp * instance_model * skin_matrix * in_position
//
// For the legacy path:
//   push.mvp        = projection * view * entity_model
//   instance_models = [ identity ]
// So instance_model is a no-op and the math collapses to the original.
//
// For the doodad path:
//   push.mvp        = projection * view   (entity_model is identity
//                                          because the entity sits at
//                                          the world origin)
//   instance_models = [ M_0, M_1, ... ]   (per-MDDF-placement TRS)
// gl_InstanceIndex selects the right matrix for the current draw call.

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

layout(set = 0, binding = 3) readonly buffer InstanceBuffer {
    mat4 instance_models[];
} instances;

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

    mat4 inst_model = instances.instance_models[gl_InstanceIndex];

    gl_Position = push.mvp * inst_model * skinned_pos;
    frag_color = in_color;
    // Normals: rotate by the entity model (legacy) or the instance
    // model (doodad path). For the legacy path push.model is the entity
    // transform and inst_model is identity; for doodads push.model is
    // identity and inst_model is the per-placement TRS. Multiplying
    // both is correct in either case.
    mat3 normal_mat = mat3(push.model) * mat3(inst_model);
    frag_normal = normal_mat * skinned_normal;
    frag_world_pos = vec3(push.model * inst_model * skinned_pos);
    frag_uv = in_uv;
}
