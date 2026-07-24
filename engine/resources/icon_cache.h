#ifndef MVE_ICON_CACHE_H
#define MVE_ICON_CACHE_H

#include "image.h"
#include "../core/device.h"
#include "../core/imgui_context.h"

#include <imgui.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace mve {

// Lazy texture cache for WoW BLP icons.
//
// Spells, items, talents, etc. reference icons by either a DBC-style path
// like "Interface\\Icons\\Spell_Fire_FlameBolt" (no .blp extension, backslash
// separators) or by an ID into SpellIcon.dbc / ItemDisplayInfo.dbc. The cache
// normalizes paths, loads the BLP from `assets/`, creates a Vulkan image,
// registers it with ImGui, and returns an `ImTextureID` the UI can drop into
// `ImGui::Image(...)`.
//
// Failures are sticky and silent — a missing icon returns `ImTextureID_Invalid`
// and the cache remembers the miss so we don't retry every frame.
//
// Lifetime: the cache owns its Images, so all `ImTextureID`s remain valid for
// the cache's lifetime. The destructor unregisters every texture from ImGui
// before dropping the images.
class IconCache {
public:
    IconCache(Device& device, ImGuiContext& imgui_ctx,
              std::string assets_root = "assets");
    ~IconCache();

    IconCache(const IconCache&) = delete;
    IconCache& operator=(const IconCache&) = delete;

    // Returns a texture for the given DBC-style path
    // (e.g. "Interface\\Icons\\Spell_Fire_FlameBolt"). Path is normalized
    // internally — case-insensitive, .blp appended if missing. Returns
    // `ImTextureID_Invalid` if the file is missing or fails to decode.
    ImTextureID Get(const std::string& dbc_path);

    // Number of icons currently loaded — useful for telemetry / debug.
    size_t LoadedCount() const { return pm_entries.size(); }

private:
    struct Entry {
        std::unique_ptr<Image> image;
        ImTextureID texture = ImTextureID_Invalid;
        bool load_failed = false;
    };

    // Convert a DBC-style path to a normalized cache key and a filesystem
    // path. Returns true on success.
    bool ResolvePath(const std::string& dbc_path,
                     std::string& out_key,
                     std::string& out_fs_path) const;

    Device& pm_device;
    ImGuiContext& pm_imgui;
    std::string pm_assets_root;

    std::unordered_map<std::string, Entry> pm_entries;
};

} // namespace mve

#endif // MVE_ICON_CACHE_H
