#version 450

// See docs/wiki/Math-Lighting.md

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

layout(set = 0, binding = 0) uniform SceneUBO {
    vec3 light_dir;
    float ambient;
    vec3 light_color;
    float light_intensity;
} scene;

layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main() {
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(scene.light_dir);
    float diffuse = max(dot(N, L), 0.0);

    vec3 tex_color = texture(tex_sampler, frag_uv).rgb;
    vec3 lighting = scene.ambient + diffuse * scene.light_intensity * scene.light_color;
    vec3 result = tex_color * frag_color * lighting;

    out_color = vec4(result, 1.0);
}
