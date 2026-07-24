#include "icon_cache.h"
#include "../formats/blp_loader.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace mve {

IconCache::IconCache(Device& device, ImGuiContext& imgui_ctx,
                     std::string assets_root)
    : pm_device{device},
      pm_imgui{imgui_ctx},
      pm_assets_root{std::move(assets_root)} {}

IconCache::~IconCache() {
    // Unregister each texture from ImGui before its backing Image goes away.
    // Order matters: ImGui still holds the descriptor set internally, freeing
    // the image first while a frame is in flight could blow up.
    for (auto& [_, entry] : pm_entries) {
        if (entry.texture != ImTextureID_Invalid) {
            pm_imgui.UnregisterTexture(entry.texture);
        }
    }
}

bool IconCache::ResolvePath(const std::string& dbc_path,
                            std::string& out_key,
                            std::string& out_fs_path) const {
    if (dbc_path.empty()) return false;

    // Normalize: lowercase, backslash -> slash, strip leading whitespace.
    std::string normalized;
    normalized.reserve(dbc_path.size() + 4);
    for (char c : dbc_path) {
        if (c == '\\') c = '/';
        normalized += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // Append .blp if missing.
    if (normalized.size() < 4 ||
        normalized.compare(normalized.size() - 4, 4, ".blp") != 0) {
        normalized += ".blp";
    }

    out_key = normalized;
    out_fs_path = pm_assets_root + "/" + normalized;
    return true;
}

ImTextureID IconCache::Get(const std::string& dbc_path) {
    std::string key, fs_path;
    if (!ResolvePath(dbc_path, key, fs_path)) {
        return ImTextureID_Invalid;
    }

    auto it = pm_entries.find(key);
    if (it != pm_entries.end()) {
        return it->second.texture;  // hit (or remembered miss)
    }

    Entry entry;

    // Filesystem path was lowercased for the cache key, but the actual file
    // on disk may be in any casing. NTFS is case-insensitive so the direct
    // open usually works, but case-sensitive filesystems (or shares mounted
    // from Linux) need a fallback. Try direct open first; if missing, scan
    // the directory case-insensitively as a last resort.
    if (!fs::exists(fs_path)) {
        fs::path target(fs_path);
        fs::path dir = target.parent_path();
        std::string want = target.filename().string();
        bool found = false;
        if (fs::is_directory(dir)) {
            for (auto& de : fs::directory_iterator(dir)) {
                std::string name = de.path().filename().string();
                if (name.size() != want.size()) continue;
                bool eq = true;
                for (size_t i = 0; i < name.size(); i++) {
                    if (std::tolower(static_cast<unsigned char>(name[i])) !=
                        std::tolower(static_cast<unsigned char>(want[i]))) {
                        eq = false;
                        break;
                    }
                }
                if (eq) {
                    fs_path = de.path().string();
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            entry.load_failed = true;
            pm_entries.emplace(key, std::move(entry));
            return ImTextureID_Invalid;
        }
    }

    try {
        auto blp = BlpLoader::LoadFile(fs_path);
        entry.image = std::make_unique<Image>(pm_device, blp);
    } catch (const std::exception&) {
        entry.load_failed = true;
        pm_entries.emplace(key, std::move(entry));
        return ImTextureID_Invalid;
    }

    entry.texture = pm_imgui.RegisterTexture(
        *entry.image->GetSampler(),
        *entry.image->GetImageView());

    ImTextureID id = entry.texture;
    pm_entries.emplace(key, std::move(entry));
    return id;
}

} // namespace mve
