#version 450

// fog_color comes in via push constant - the bg pipeline doesn't have
// a descriptor set layout (it draws a fullscreen triangle with no
// resources) and adding one just for one vec3 isn't worth it.
// Anchoring sky to fog_color means distant fogged geometry blends
// into the sky seamlessly instead of meeting a hardcoded blue.
layout(push_constant) uniform Push {
    vec3 fog_color;
} push;

layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

void main() {
    // Vertical gradient anchored to fog_color. Horizon = fog tint
    // (matches distant geometry). Top of sky = fog * 0.75 (slightly
    // darker overhead, like overcast cloud cover). Below horizon =
    // fog * 0.4 (ground-shadowed strip below the camera).
    float t = frag_uv.y;
    vec3 ground_color  = push.fog_color * 0.40;
    vec3 horizon_color = push.fog_color;
    vec3 sky_color     = push.fog_color * 0.75;

    float horizon = 0.4;
    vec3 color;
    if (t < horizon) {
        float gt = t / horizon;
        color = mix(ground_color, horizon_color, gt * gt);
    } else {
        float st = (t - horizon) / (1.0 - horizon);
        color = mix(horizon_color, sky_color, st);
    }
    out_color = vec4(color, 1.0);
}
