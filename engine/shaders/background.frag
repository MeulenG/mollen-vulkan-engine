#version 450

// WoW client sky-cone gradient (LightIntBand rows 2-5 + 7). The
// engine pushes the canonical 5-band Elwynn-noon colors as a single
// 80-byte push constant; this shader maps the fullscreen-triangle UV
// onto an elevation band 0..1 (0 = below horizon, 0.5 = horizon, 1.0
// = zenith) and blends between the bands at the documented elevation
// breakpoints.
//
// Elevation breakpoints from wowdev.wiki/Day_night_cycle "Sky Cone
// Method 2":
//
//   elev > 0.714  ->  sky_middle -> sky_top    (zenith)
//   elev > 0.547  ->  sky_band1  -> sky_middle
//   elev > 0.513  ->  sky_band2  -> sky_band1
//   elev > 0.500  ->  fog_color  -> sky_band2  (smog hint)
//   else          ->  fog_color                (below horizon)
//
// The canonical breakpoints assume a 3D sky-dome mesh that wraps a
// full hemisphere. The "interesting" detail (band1, band2, smog) all
// happens in the 0.500-0.547 range - only 4.7% of the elevation
// range, but it covers a large visual area on a real sky dome
// because the dome is angularly large at the horizon.
//
// On a flat 2D backdrop with UV.y mapped 1:1 to elevation, those
// same breakpoints compress band1+band2 into ~3% of the screen,
// producing a visible hard stripe at the horizon and an unbalanced
// gradient overall. To match the WoW client's visual proportions
// on a flat backdrop, we REMAP UV.y to elevation so the per-band
// screen area approximates a hemispheric dome's per-band area:
//
//   uv.y = 0.00..0.50  ->  elev 0.00..0.50  (below horizon, fog)
//   uv.y = 0.50..0.55  ->  elev 0.50..0.513 (smog)
//   uv.y = 0.55..0.65  ->  elev 0.513..0.547 (band2)
//   uv.y = 0.65..0.92  ->  elev 0.547..0.714 (band1 -> middle)
//   uv.y = 0.92..1.00  ->  elev 0.714..1.000 (middle -> top)
//
// This is mathematically equivalent to applying a tan-elevation
// remap (a uniform-screen-Y has non-uniform angular elevation under
// perspective), which is what a 3D sky dome inherently produces.

layout(push_constant) uniform Push {
    vec3 sky_top;
    vec3 sky_middle;
    vec3 sky_band1;
    vec3 sky_band2;
    vec3 fog_color;
} push;

layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

// Piecewise-linear UV -> elevation remap to match the per-band
// screen-area proportions of a hemispheric sky dome on a 2D
// backdrop. Each (uv, elev) pair is a breakpoint of the remap.
float UvToElev(float uv) {
    if      (uv >= 0.92) return mix(0.714, 1.000, (uv - 0.92) / 0.08);
    else if (uv >= 0.65) return mix(0.547, 0.714, (uv - 0.65) / 0.27);
    else if (uv >= 0.55) return mix(0.513, 0.547, (uv - 0.55) / 0.10);
    else if (uv >= 0.50) return mix(0.500, 0.513, (uv - 0.50) / 0.05);
    else                 return mix(0.000, 0.500, uv / 0.50);
}

void main() {
    // Vulkan default: NDC y points down, so the fullscreen triangle's
    // UV.y = 0 lands at the TOP of the screen and UV.y = 1 at the
    // bottom. We want elevation = 1.0 (zenith) at the top, so flip.
    float e = UvToElev(1.0 - frag_uv.y);
    vec3 color;
    if (e >= 0.714) {
        color = mix(push.sky_middle, push.sky_top,
                    smoothstep(0.714, 1.000, e));
    } else if (e >= 0.547) {
        color = mix(push.sky_band1, push.sky_middle,
                    smoothstep(0.547, 0.714, e));
    } else if (e >= 0.513) {
        color = mix(push.sky_band2, push.sky_band1,
                    smoothstep(0.513, 0.547, e));
    } else if (e >= 0.500) {
        color = mix(push.fog_color, push.sky_band2,
                    smoothstep(0.500, 0.513, e));
    } else {
        // Below horizon: pure fog tint. Distant terrain fogs to this
        // color, so the seam between terrain and sky is invisible.
        color = push.fog_color;
    }
    out_color = vec4(color, 1.0);
}
