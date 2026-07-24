#ifndef MVE_SPELL_EDITOR_SYSTEM_H
#define MVE_SPELL_EDITOR_SYSTEM_H

#include "../db/db_connection.h"
#include "../resources/icon_cache.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace mve {

// Curated Spell.dbc editor — the friendly "spell card" view (vs. the generic
// table in DbcBrowserSystem). Optimized for the workflow "find Fireball rank
// 3, change a value, save".
//
// Left pane: icon-thumbnail spell list with search.
// Right pane: detail card showing icon + name + rank + key editable fields
//             + collapsible advanced sections.
//
// Reads + writes via DbConnection. Pulls icons via IconCache. Spell.dbc has
// 50k+ rows so the table is fetched once lazily and filtered in-memory.
class SpellEditorSystem {
public:
    SpellEditorSystem(DbConnection& db, IconCache& icons);

    void Update();

private:
    // Pre-fetch the SpellIcon table once so icon_id -> path resolves locally
    // without round-tripping the DB per render frame.
    void EnsureLoaded();

    // Resolve column indices once after the spell table is fetched. Storing
    // indices avoids hashing column names every frame.
    void CacheColumnIndices();

    void DrawFinder();
    void DrawDetail();

    // Convenience accessors over the cached row data — return defaults on miss.
    std::string GetCell(size_t row_index, int col_index) const;
    int64_t GetCellInt(size_t row_index, int col_index) const;

    // Resolve a spell row's icon path through SpellIcon.dbc lookup, then
    // through IconCache. Returns ImTextureID_Invalid if anything is missing.
    ImTextureID IconFor(size_t row_index);

    // Commit a single cell edit. Refreshes the in-memory row on success.
    void Commit(int64_t spell_id, size_t row_index,
                const std::string& column,
                const std::string& value,
                DbcFieldType type);

    DbConnection& pm_db;
    IconCache& pm_icons;

    DbConnection::Table pm_spells;
    bool pm_loaded = false;
    std::string pm_load_error;

    // Column index cache (resolved once at load time).
    struct Cols {
        int id          = -1;
        int name        = -1;  // spell_name_en_us
        int rank        = -1;  // rank_en_us
        int description = -1;  // description_en_us
        int icon_id     = -1;  // spell_icon_id
        int mana_cost   = -1;
        int cast_time_index = -1;
        int range_index = -1;
        int recovery_time = -1;
        int school_mask = -1;
        int spell_family = -1;
        int effect_1    = -1;
        int effect_base_points_1 = -1;
        int effect_die_sides_1 = -1;
    };
    Cols pm_cols;

    // SpellIcon.id -> icon path (DBC-style, no .blp suffix). Loaded once.
    std::unordered_map<int64_t, std::string> pm_icon_paths;

    // Currently selected spell row (-1 = none).
    int pm_selected_row = -1;

    // Pre-computed lowercase names for filter matching. Same length as rows.
    std::vector<std::string> pm_lowercase_names;

    char pm_filter[64] = {0};

    // Per-cell edit buffers, keyed by column index. Reset when selection
    // changes. Indices we edit are small so a flat map is fine.
    std::unordered_map<int, std::string> pm_edit_buffers;
    std::string pm_last_error;
};

} // namespace mve

#endif // MVE_SPELL_EDITOR_SYSTEM_H
