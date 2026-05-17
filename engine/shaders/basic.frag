#version 450

// See docs/wiki/Math-Lighting.md

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_world_pos;
layout(location = 3) in vec2 frag_uv;

// std140 layout matches engine::SceneUBO. Trailing fields are fog +
// camera position for the linear-distance fog calculation below.
layout(set = 0, binding = 0) uniform SceneUBO {
    vec3 light_dir;
    float ambient;
    vec3 light_color;
    float light_intensity;
    vec3 fog_color;
    float fog_start;
    vec3 camera_pos;
    float fog_end;
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

    // Linear depth fog. Distance from camera in world space; fade
    // toward fog_color between fog_start and fog_end. Smooths the
    // hard far-plane cutoff into an atmospheric haze.
    float dist = length(frag_world_pos - scene.camera_pos);
    float fog = clamp((dist - scene.fog_start) /
                       (scene.fog_end - scene.fog_start), 0.0, 1.0);
    result = mix(result, scene.fog_color, fog);

    out_color = vec4(result, 1.0);
}
