#include "dbc_registry.h"
#include "schema_registry.h"

#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

namespace mve {

DbcRegistry::DbcRegistry(fs::path dbc_dir)
    : pm_dbc_dir{std::move(dbc_dir)} {
    if (!fs::exists(pm_dbc_dir) || !fs::is_directory(pm_dbc_dir)) {
        return;
    }

    for (auto& dir_entry : fs::directory_iterator(pm_dbc_dir)) {
        if (!dir_entry.is_regular_file()) continue;

        auto ext = dir_entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != ".dbc") continue;

        std::string name = dir_entry.path().stem().string();

        auto entry = std::make_unique<Entry>();
        entry->name = name;
        entry->path = dir_entry.path();
        entry->schema = GetSchema(name.c_str());

        pm_names.push_back(name);
        pm_entries[name] = std::move(entry);
    }

    std::sort(pm_names.begin(), pm_names.end());
}

DbcRegistry::Entry* DbcRegistry::Load(const std::string& name) {
    auto it = pm_entries.find(name);
    if (it == pm_entries.end()) return nullptr;

    Entry* entry = it->second.get();
    if (entry->file || entry->load_failed) return entry;

    std::ifstream file(entry->path, std::ios::binary | std::ios::ate);
    if (!file) {
        entry->load_failed = true;
        return entry;
    }

    auto size = static_cast<std::streamsize>(file.tellg());
    file.seekg(0, std::ios::beg);

    entry->bytes.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(entry->bytes.data()), size)) {
        entry->bytes.clear();
        entry->load_failed = true;
        return entry;
    }

    auto dbc = std::make_unique<DbcFile>();
    if (!dbc->Load(entry->bytes.data(), static_cast<uint32_t>(entry->bytes.size()))) {
        entry->bytes.clear();
        entry->load_failed = true;
        return entry;
    }

    entry->file = std::move(dbc);
    return entry;
}

bool DbcRegistry::HasSchema(const std::string& name) const {
    auto it = pm_entries.find(name);
    return it != pm_entries.end() && it->second->schema != nullptr;
}

} // namespace mve
