#version 450

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

// Storage buffer for bone matrices.
// Storage buffers can hold much more data than uniform buffers (128KB+ vs 16KB).
// We need up to 256 bones * 64 bytes each = 16KB, so UBO would work too,
// but storage buffer is more flexible for future use.
layout(set = 0, binding = 2) readonly buffer BoneBuffer {
    mat4 bone_matrices[];
} bones;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_world_pos;
layout(location = 3) out vec2 frag_uv;

void main() {
    // --- Skinning ---
    // Each vertex is influenced by up to 4 bones.
    // We compute a weighted blend of all bone transforms:
    //
    //   skin_matrix = w0 * bone[i0] + w1 * bone[i1] + w2 * bone[i2] + w3 * bone[i3]
    //
    // Each bone_matrices[i] already contains: current_world * inverse_bind
    // So multiplying by it transforms the vertex from bind pose to animated pose.
    //
    // If bone_weights = (1, 0, 0, 0), only one bone affects the vertex (rigid).
    // Near joints, weights are blended (e.g., 0.5, 0.5, 0, 0) for smooth deformation.

    mat4 skin_matrix =
        in_bone_weights.x * bones.bone_matrices[in_bone_indices.x] +
        in_bone_weights.y * bones.bone_matrices[in_bone_indices.y] +
        in_bone_weights.z * bones.bone_matrices[in_bone_indices.z] +
        in_bone_weights.w * bones.bone_matrices[in_bone_indices.w];

    // Apply skinning then model transform
    vec4 skinned_pos = skin_matrix * vec4(in_position, 1.0);

    // Normal transform: use mat3 of the skin matrix (strips translation).
    // For non-uniform scale, you'd need the inverse transpose,
    // but for skeletal animation the scale is typically uniform.
    vec3 skinned_normal = mat3(skin_matrix) * in_normal;

    gl_Position = push.mvp * skinned_pos;
    frag_color = in_color;
    frag_normal = mat3(push.model) * skinned_normal;
    frag_world_pos = vec3(push.model * skinned_pos);
    frag_uv = in_uv;
}
