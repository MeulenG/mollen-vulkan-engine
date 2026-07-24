#include "editor_style.h"
#include "icons_fork_awesome.h"

#include <imgui.h>

#include <filesystem>
#include <string>

namespace mve::editor_style {

namespace {

// sRGB hex -> ImVec4. ImGui's Vulkan backend submits these as sRGB encoded
// in the framebuffer; keep the conversion trivial.
constexpr ImVec4 RGB(int r, int g, int b, float a = 1.0f) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a);
}

// ---- Palette --------------------------------------------------------------
// Neutral dark slate backgrounds + a warm gold accent. WoW-flavored without
// being garish. Status colors are deliberately muted so the DBC browser
// doesn't look like a Christmas tree when many rows are colored at once.
constexpr ImVec4 accent_gold     = RGB(0xD4, 0xA2, 0x4B);
constexpr ImVec4 accent_gold_dim = RGB(0xA0, 0x7A, 0x38);
constexpr ImVec4 accent_orange   = RGB(0xF5, 0x87, 0x2E);

constexpr ImVec4 bg_window       = RGB(0x1E, 0x22, 0x2A);
constexpr ImVec4 bg_child        = RGB(0x22, 0x27, 0x30);
constexpr ImVec4 bg_popup        = RGB(0x26, 0x2B, 0x35);
constexpr ImVec4 bg_frame        = RGB(0x2D, 0x33, 0x3E);
constexpr ImVec4 bg_frame_hover  = RGB(0x3A, 0x41, 0x50);
constexpr ImVec4 bg_frame_active = RGB(0x46, 0x4E, 0x5F);
constexpr ImVec4 bg_titlebar     = RGB(0x17, 0x1A, 0x21);
constexpr ImVec4 bg_titlebar_act = RGB(0x22, 0x27, 0x30);

constexpr ImVec4 border          = RGB(0x40, 0x46, 0x52);
constexpr ImVec4 border_shadow   = RGB(0x00, 0x00, 0x00, 0.0f);

constexpr ImVec4 text            = RGB(0xDC, 0xD7, 0xCB);
constexpr ImVec4 text_dim        = RGB(0x82, 0x80, 0x78);
constexpr ImVec4 text_selection  = RGB(0xD4, 0xA2, 0x4B, 0.35f);

constexpr ImVec4 success_green   = RGB(0x6F, 0xA4, 0x6A);
constexpr ImVec4 warning_amber   = RGB(0xD4, 0xA2, 0x4B);
constexpr ImVec4 error_red       = RGB(0xB5, 0x52, 0x4A);

} // namespace

void Apply(::ImGuiContext& /*ctx*/) {
    ImGuiStyle& style = ImGui::GetStyle();

    // ---- Spacing & rounding -----------------------------------------------
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.FramePadding      = ImVec2(8, 4);
    style.WindowPadding     = ImVec2(12, 12);
    style.CellPadding       = ImVec2(6, 3);
    style.IndentSpacing     = 20.0f;
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 10.0f;

    style.WindowRounding    = 6.0f;
    style.ChildRounding     = 4.0f;
    style.PopupRounding     = 6.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 4.0f;
    style.ScrollbarRounding = 4.0f;

    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;

    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);

    // ---- Palette ----------------------------------------------------------
    ImGui::StyleColorsDark();
    ImVec4* c = style.Colors;

    c[ImGuiCol_Text]                  = text;
    c[ImGuiCol_TextDisabled]          = text_dim;
    c[ImGuiCol_TextSelectedBg]        = text_selection;

    c[ImGuiCol_WindowBg]              = bg_window;
    c[ImGuiCol_ChildBg]               = bg_child;
    c[ImGuiCol_PopupBg]               = bg_popup;
    c[ImGuiCol_Border]                = border;
    c[ImGuiCol_BorderShadow]          = border_shadow;

    c[ImGuiCol_FrameBg]               = bg_frame;
    c[ImGuiCol_FrameBgHovered]        = bg_frame_hover;
    c[ImGuiCol_FrameBgActive]         = bg_frame_active;

    c[ImGuiCol_TitleBg]               = bg_titlebar;
    c[ImGuiCol_TitleBgActive]         = bg_titlebar_act;
    c[ImGuiCol_TitleBgCollapsed]      = bg_titlebar;
    c[ImGuiCol_MenuBarBg]             = bg_titlebar_act;

    c[ImGuiCol_ScrollbarBg]           = bg_window;
    c[ImGuiCol_ScrollbarGrab]         = bg_frame_hover;
    c[ImGuiCol_ScrollbarGrabHovered]  = bg_frame_active;
    c[ImGuiCol_ScrollbarGrabActive]   = accent_gold_dim;

    c[ImGuiCol_CheckMark]             = accent_gold;
    c[ImGuiCol_SliderGrab]            = accent_gold_dim;
    c[ImGuiCol_SliderGrabActive]      = accent_gold;

    c[ImGuiCol_Button]                = bg_frame;
    c[ImGuiCol_ButtonHovered]         = bg_frame_hover;
    c[ImGuiCol_ButtonActive]          = accent_gold_dim;

    c[ImGuiCol_Header]                = ImVec4(accent_gold_dim.x, accent_gold_dim.y, accent_gold_dim.z, 0.40f);
    c[ImGuiCol_HeaderHovered]         = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.55f);
    c[ImGuiCol_HeaderActive]          = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.75f);

    c[ImGuiCol_Separator]             = border;
    c[ImGuiCol_SeparatorHovered]      = accent_gold_dim;
    c[ImGuiCol_SeparatorActive]       = accent_gold;

    c[ImGuiCol_ResizeGrip]            = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.20f);
    c[ImGuiCol_ResizeGripHovered]     = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.50f);
    c[ImGuiCol_ResizeGripActive]      = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.85f);

    c[ImGuiCol_Tab]                   = bg_titlebar;
    c[ImGuiCol_TabHovered]            = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.60f);
    c[ImGuiCol_TabActive]             = bg_titlebar_act;
    c[ImGuiCol_TabUnfocused]          = bg_titlebar;
    c[ImGuiCol_TabUnfocusedActive]    = bg_child;

    c[ImGuiCol_DockingPreview]        = ImVec4(accent_gold.x, accent_gold.y, accent_gold.z, 0.50f);
    c[ImGuiCol_DockingEmptyBg]        = bg_window;

    c[ImGuiCol_TableHeaderBg]         = bg_titlebar_act;
    c[ImGuiCol_TableBorderStrong]     = border;
    c[ImGuiCol_TableBorderLight]      = ImVec4(border.x, border.y, border.z, 0.50f);
    c[ImGuiCol_TableRowBg]            = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(1, 1, 1, 0.03f);

    c[ImGuiCol_NavHighlight]          = accent_gold;
    c[ImGuiCol_DragDropTarget]        = accent_orange;

    // Status colors parked on the Plot slots so they're inspectable via
    // ImGui::PushStyleColor and discoverable via the palette. Editor code
    // can later wire dedicated accessors if needed.
    c[ImGuiCol_PlotLines]             = success_green;
    c[ImGuiCol_PlotLinesHovered]      = warning_amber;
    c[ImGuiCol_PlotHistogram]         = error_red;
    c[ImGuiCol_PlotHistogramHovered]  = accent_orange;
}

namespace {

// Find a font in the obvious places. Returns empty string on miss.
std::string FindFont(const char* filename) {
    namespace fs = std::filesystem;
    const fs::path candidates[] = {
        fs::path("engine") / "assets" / "fonts" / filename,
        fs::path("assets") / "fonts" / filename,
        fs::path("..") / "engine" / "assets" / "fonts" / filename,
        fs::path("..") / ".." / "engine" / "assets" / "fonts" / filename,
        fs::path("..") / ".." / ".." / "engine" / "assets" / "fonts" / filename,
    };
    std::error_code ec;
    for (const auto& p : candidates) {
        if (fs::exists(p, ec)) return p.string();
    }
    return {};
}

} // namespace

void LoadFonts(::ImFontAtlas& atlas) {
    constexpr float kBodyPixelSize = 14.0f;
    constexpr float kIconPixelSize = 13.0f;

    const std::string inter = FindFont("Inter-Regular.ttf");
    const std::string icons = FindFont("ForkAwesome.ttf");

    if (!inter.empty()) {
        atlas.AddFontFromFileTTF(inter.c_str(), kBodyPixelSize);
    } else {
        // Keeps the editor functional if fonts are missing. The visual
        // difference vs. Inter is the user-facing signal that they should
        // be fetched.
        atlas.AddFontDefault();
    }

    if (!icons.empty()) {
        // Merge Fork Awesome's glyph range into the SAME font, so
        // ImGui::Text(ICON_FA_SEARCH " Filter") works without push/pop.
        static const ImWchar kIconRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

        ImFontConfig cfg;
        cfg.MergeMode = true;
        cfg.PixelSnapH = true;
        cfg.GlyphOffset = ImVec2(0.0f, 1.0f);

        atlas.AddFontFromFileTTF(icons.c_str(), kIconPixelSize, &cfg, kIconRange);
    }

    atlas.Build();
}

} // namespace mve::editor_style
