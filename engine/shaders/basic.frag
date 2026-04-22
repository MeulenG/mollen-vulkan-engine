#version 450

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

// Uniform buffer — lighting parameters, updated per frame
layout(set = 0, binding = 0) uniform SceneUBO {
    vec3 light_dir;       // direction TO the light (normalized)
    float ambient;        // minimum brightness (0.0 - 1.0)
    vec3 light_color;     // light RGB (usually white)
    float light_intensity; // multiplier on diffuse
} scene;

// Texture sampler — the texture image bound at set 0, binding 1
layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main() {
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(scene.light_dir);

    // Lambert diffuse: dot(N, L) = cos(angle between normal and light)
    // max clamps negative values (surfaces facing away from light)
    float diffuse = max(dot(N, L), 0.0);

    // Sample texture at interpolated UV coordinate
    vec3 tex_color = texture(tex_sampler, frag_uv).rgb;

    // Final color = texture * vertex_color * (ambient + diffuse * intensity)
    // vertex_color acts as a tint/multiplier (WoW uses this for baked lighting)
    vec3 lighting = scene.ambient + diffuse * scene.light_intensity * scene.light_color;
    vec3 result = tex_color * frag_color * lighting;

    out_color = vec4(result, 1.0);
}
