#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;

layout(location = 0) out vec4 out_color;

void main() {
    // Directional light
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 normal = normalize(frag_normal);

    float ambient = 0.15;
    float diffuse = max(dot(normal, light_dir), 0.0);

    vec3 lit_color = frag_color * (ambient + diffuse * 0.85);

    out_color = vec4(lit_color, 1.0);
}
