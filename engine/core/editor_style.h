#ifndef MVE_EDITOR_STYLE_H
#define MVE_EDITOR_STYLE_H

// Forward declares — fully-qualified so they don't collide with `mve::ImGuiContext`
// (the project's wrapper class in engine/core/imgui_context.h).
struct ImGuiContext;
struct ImFontAtlas;

namespace mve::editor_style {

// Apply the editor-wide ImGui theme: palette, spacing, rounding.
// Call once after ImGui::CreateContext() and before any ImGui rendering.
void Apply(::ImGuiContext& ctx);

// Load the Inter base font and merge Fork Awesome icons into the same atlas.
// Looks for fonts under several candidate directories (see implementation).
// Silently falls back to the default proggy font if neither is found, so
// the editor still launches on a misconfigured developer machine.
void LoadFonts(::ImFontAtlas& atlas);

} // namespace mve::editor_style

#endif // MVE_EDITOR_STYLE_H
