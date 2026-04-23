#version 450

layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

void main() {
    // Vertical gradient: dark at bottom, lighter blue at top
    // UV.y: 0 = bottom of screen, 1 = top
    float t = frag_uv.y;

    // Ground color (dark earthy tone) below horizon, sky gradient above
    vec3 ground_color = vec3(0.15, 0.13, 0.11);
    vec3 horizon_color = vec3(0.35, 0.45, 0.55);
    vec3 sky_color = vec3(0.15, 0.25, 0.45);

    // Horizon at roughly 40% up the screen
    float horizon = 0.4;

    vec3 color;
    if (t < horizon) {
        // Below horizon: ground to horizon
        float gt = t / horizon;
        color = mix(ground_color, horizon_color, gt * gt);
    } else {
        // Above horizon: horizon to sky
        float st = (t - horizon) / (1.0 - horizon);
        color = mix(horizon_color, sky_color, st);
    }

    out_color = vec4(color, 1.0);
}
