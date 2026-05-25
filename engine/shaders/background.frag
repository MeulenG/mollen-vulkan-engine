#version 450

// WoW client sky-cone gradient (LightIntBand rows 2-5 + 7) + two
// procedural cloud layers (LightIntBand rows 11 + 12, density from
// LightFloatBand row 3). The engine pushes 128 bytes total: 5 sky
// band vec4s, 2 cloud ambient vec4s, 1 cloud-animation vec4.
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
// Cloud compositing math:
//   The clouds are sampled on a virtual plane above the camera. To
//   make clouds appear smaller / more compressed near the horizon
//   (mimicking a dome's foreshortening), we divide screen UV by
//   (elev - 0.45) before sampling. Near the horizon (elev ~ 0.5),
//   this divisor is small, so the UVs span a HUGE area of noise
//   space, packing many cloud features into few pixels. Near the
//   zenith (elev ~ 1.0), the divisor is ~0.55, so each pixel
//   samples a smaller region of noise space, making cloud puffs
//   appear larger. This is the same projection trick a real
//   sky-dome mesh would produce under perspective.

layout(push_constant) uniform Push {
    vec3 sky_top;          float _pad0;
    vec3 sky_middle;       float _pad1;
    vec3 sky_band1;        float _pad2;
    vec3 sky_band2;        float _pad3;
    vec3 fog_color;        float _pad4;
    vec3 cloud_layer1_amb; float _pad5;
    vec3 cloud_layer2_amb; float _pad6;
    // cloud_anim.x = cloud_density [0,1]
    // cloud_anim.y = game time seconds (continuous, no wrap)
    // cloud_anim.z = celestial_glow_through (sun brightness through clouds)
    vec4 cloud_anim;
} push;

layout(location = 0) in vec2 frag_uv;
layout(location = 0) out vec4 out_color;

// IQ-style 2D value noise via integer hash. Fast, no texture sample,
// produces a smooth field suitable for compositing as cloud density.
float hash2(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float vnoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    // Quintic smoothstep for C2 continuity - prevents lattice banding
    // that you'd see with cubic on slowly-scrolling clouds.
    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    float a = hash2(i + vec2(0, 0));
    float b = hash2(i + vec2(1, 0));
    float c = hash2(i + vec2(0, 1));
    float d = hash2(i + vec2(1, 1));
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// 4-octave fractional Brownian motion. Each octave doubles the
// frequency and halves the amplitude - produces the familiar
// "billowy cumulus" silhouette WoW uses.
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(0.80, 0.60, -0.60, 0.80);  // gentle per-octave rotation, kills axis bias
    for (int i = 0; i < 4; ++i) {
        v += a * vnoise(p);
        p = rot * p * 2.0;
        a *= 0.5;
    }
    return v;
}

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

    // Base sky cone gradient.
    vec3 sky_color;
    if (e >= 0.714) {
        sky_color = mix(push.sky_middle, push.sky_top,
                        smoothstep(0.714, 1.000, e));
    } else if (e >= 0.547) {
        sky_color = mix(push.sky_band1, push.sky_middle,
                        smoothstep(0.547, 0.714, e));
    } else if (e >= 0.513) {
        sky_color = mix(push.sky_band2, push.sky_band1,
                        smoothstep(0.513, 0.547, e));
    } else if (e >= 0.500) {
        sky_color = mix(push.fog_color, push.sky_band2,
                        smoothstep(0.500, 0.513, e));
    } else {
        // Below horizon: pure fog tint. Distant terrain fogs to this
        // color, so the seam between terrain and sky is invisible.
        sky_color = push.fog_color;
    }

    // Procedural cloud layer (only above horizon).
    if (e > 0.500) {
        float t = push.cloud_anim.y;
        float density = push.cloud_anim.x;

        // Center horizontally then divide by elev-factor for the
        // dome-style foreshortening described in the header comment.
        float elev_factor = max(e - 0.45, 0.05);
        vec2 uv_c = vec2(frag_uv.x - 0.5, frag_uv.y);

        // Two cloud layers at different scales and scroll speeds.
        // Layer 1: large slow-drifting cumulus.
        // Layer 2: smaller, faster wisps blown by upper winds.
        vec2 wind1 = vec2(0.012, 0.004);
        vec2 wind2 = vec2(0.020, 0.007);
        vec2 cuv1 = (uv_c * 2.5 + wind1 * t) / elev_factor;
        vec2 cuv2 = (uv_c * 5.5 + wind2 * t) / elev_factor;

        float c1 = fbm(cuv1);
        float c2 = fbm(cuv2 * 0.7);

        // Cloud coverage: cloud_density biases the noise threshold so
        // overcast (density=1) covers most of the sky and clear
        // (density=0) leaves almost none. The +0.15 bias keeps a few
        // wisps visible even at density=0; pure clear days look
        // unnaturally empty otherwise.
        float coverage = smoothstep(0.55 - density * 0.55,
                                     0.85 - density * 0.35,
                                     c1);
        float wisp     = smoothstep(0.60, 0.85, c2) * density;

        // Color the clouds: blend the two layer-ambient tints by the
        // wisp factor. Layer 1 is the dominant base tint, layer 2 is
        // the brighter edge highlight.
        vec3 cloud_color = mix(push.cloud_layer1_amb,
                               push.cloud_layer2_amb,
                               wisp);

        // Tint clouds toward fog_color near the horizon so the seam
        // between distant fog-tinted terrain and the sky-side clouds
        // is invisible. Without this you get the "WoW clouds floating
        // above a foggy horizon line" wrong-looking break.
        float horizon_blend = smoothstep(0.500, 0.620, e);
        cloud_color = mix(push.fog_color, cloud_color, horizon_blend);

        // Composite clouds over sky. Coverage * horizon_blend ensures
        // clouds gradually fade IN as elevation increases past the
        // horizon, eliminating any popping at the boundary.
        sky_color = mix(sky_color, cloud_color, coverage * horizon_blend);
    }

    out_color = vec4(sky_color, 1.0);
}
