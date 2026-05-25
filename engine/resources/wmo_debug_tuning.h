#ifndef MVE_WMO_DEBUG_TUNING_H
#define MVE_WMO_DEBUG_TUNING_H

namespace mve {

// Runtime-adjustable parameters for the WMO model-matrix construction.
// Owned globally so both render_system (rebuilds matrices per frame)
// and editor_ui_system (writes via sliders) can reach them without
// threading state through Scene. Changes take effect the very next
// frame because render_system reads these every draw.
//
// Defaults reflect the empirically-validated transform for our basis
// (Y-up, engine.X = server.X, engine.Z = server.Y). With these on,
// asymmetric WMOs like Stormwind's flight master tower render with
// the holes on the correct side. The full per-frame model matrix is:
//
//   M = T(engine_pos) * R(q_yaw * q_pitch * q_roll) * swapYZ * mirror
//
// where the active rotation values are (with defaults):
//   yaw   = -rot_y                around (0,1,0)  [up]
//   pitch = +rot_x                around (0,0,1)
//   roll  = +rot_z                around (1,0,0)
// and swapYZ * mirror_x composes to the rotation (x,y,z) -> (-x,z,y),
// the correct Z-up-to-Y-up basis change for our engine (det = +1).
//
// Sliders are kept around for future WMO debugging; tweaking them
// changes WMO transforms live without recompilation.
struct WmoDebugTuning {
    // Extra yaw rotation in degrees added on top of rot_y. Reserved
    // for cases where individual WMO sets need a heading nudge.
    float yaw_offset_deg = 0.0f;

    // Use -rot_y instead of +rot_y. Required in our basis because the
    // SwapYZ * mirror_x composition is a rotation around our up axis
    // in the opposite direction to WoWee's renderZ rotation.
    bool yaw_sign_flip = true;

    // Apply MODF rot_x / rot_z directly (no sign flip). The empirical
    // test showed positive signs are correct for our basis, even
    // though WoWee uses negative signs in their Z-up engine.
    bool pitch_sign_flip = false;
    bool roll_sign_flip  = false;

    // Reserved: swap pitch and roll AXES (not values). Empirically
    // not needed; the canonical axes from the similarity-transform
    // derivation are correct.
    bool swap_pitch_roll_axes = false;

    // Extra mirror along each engine axis (negate that component as
    // innermost factor in the model matrix). mirror_x ON combined
    // with swapYZ composes to the clean basis-change rotation
    // (x,y,z) -> (-x,z,y).
    bool mirror_x = true;
    bool mirror_y = false;
    bool mirror_z = false;
};

// Process-wide tunables. Defined in editor_ui_system.cpp.
extern WmoDebugTuning g_wmo_debug;

} // namespace mve

#endif // MVE_WMO_DEBUG_TUNING_H
