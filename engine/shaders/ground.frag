#version 450

layout(location = 0) in vec3 frag_world_pos;
layout(location = 0) out vec4 out_color;

void main() {
    // Soft grid pattern on the ground plane
    vec2 grid_pos = frag_world_pos.xz; // XZ is the ground plane (after coordinate conversion)

    // Grid lines every 1 unit, with anti-aliased edges
    vec2 grid = abs(fract(grid_pos - 0.5) - 0.5);
    vec2 line_width = fwidth(grid_pos);
    vec2 grid_aa = smoothstep(vec2(0.0), line_width * 1.5, grid);
    float grid_factor = min(grid_aa.x, grid_aa.y);

    // Base ground color with subtle grid
    vec3 base_color = vec3(0.20, 0.22, 0.18); // dark earthy green-grey
    vec3 line_color = vec3(0.25, 0.27, 0.23);

    vec3 color = mix(line_color, base_color, grid_factor);

    // Fade out at distance for a soft edge
    float dist = length(frag_world_pos.xz);
    float fade = 1.0 - smoothstep(8.0, 15.0, dist);

    out_color = vec4(color, fade);
}
