#ifndef MVE_DBC_REGISTRY_H
#define MVE_DBC_REGISTRY_H

#include "dbc_file.h"
#include "dbc_schema.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace mve {

// Scans a directory for .dbc files at construction, then loads them
// lazily on first request. Loaded files (and their backing byte buffers)
// are cached for the lifetime of the registry.
//
// Schemas are resolved via the global schema_registry. A DBC without a
// known schema is still listed and loadable; the browser just shows raw
// columns for it.
class DbcRegistry {
public:
    struct Entry {
        std::string name;                   // e.g. "CreatureDisplayInfo"
        std::filesystem::path path;         // absolute path to the .dbc on disk
        const DbcSchema* schema = nullptr;  // null if no schema is registered

        // Populated on first Load() call. Bytes must live as long as `file`
        // because DbcFile stores raw pointers into them.
        std::vector<uint8_t> bytes;
        std::unique_ptr<DbcFile> file;
        bool load_failed = false;
    };

    explicit DbcRegistry(std::filesystem::path dbc_dir);

    // Names of all .dbc files found on disk, sorted alphabetically.
    const std::vector<std::string>& AvailableNames() const { return pm_names; }

    // The directory that was scanned.
    const std::filesystem::path& DbcDir() const { return pm_dbc_dir; }

    // Returns the entry for `name`, parsing the file on first call.
    // Returns nullptr if the file isn't in the registry. Returns a non-null
    // entry with `load_failed = true` if parsing failed.
    Entry* Load(const std::string& name);

    // True if a schema is registered for `name`. The DBC may still be
    // listed and loadable without one.
    bool HasSchema(const std::string& name) const;

private:
    std::filesystem::path pm_dbc_dir;
    std::vector<std::string> pm_names;
    std::unordered_map<std::string, std::unique_ptr<Entry>> pm_entries;
};

} // namespace mve

#endif // MVE_DBC_REGISTRY_H
