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
        int name        = -1;
        int rank        = -1;
        int description = -1;
        int icon_id     = -1;
        // Mana / power costs — Spell.dbc uses several columns; non-zero
        // entries are summed/combined at runtime by the WoW client.
        int mana_cost            = -1;
        int mana_cost_percentage = -1;
        int mana_cost_per_level  = -1;
        int mana_per_second      = -1;
        int mana_per_second_per_level = -1;
        int power_type           = -1;
        // Timing FKs — small lookup DBCs.
        int cast_time_index = -1;
        int range_index     = -1;
        int duration_index  = -1;
        int recovery_time   = -1;
        // Other identity / classification.
        int school_mask  = -1;
        int spell_family = -1;
        int max_targets  = -1;
        int proc_chance  = -1;
        // Effects (slot 1 — slot 2/3 added in next branch).
        int effect_1                = -1;
        int effect_base_points_1    = -1;
        int effect_die_sides_1      = -1;
        int effect_amplitude_1      = -1;  // tick period in ms for periodic effects
        int effect_radius_index_1   = -1;
        // Per-effect (we read 2/3 for description token substitution even though
        // we don't render their section yet — $s2, $o3 might be referenced).
        int effect_2                = -1;
        int effect_base_points_2    = -1;
        int effect_die_sides_2      = -1;
        int effect_amplitude_2      = -1;
        int effect_radius_index_2   = -1;
        int effect_3                = -1;
        int effect_base_points_3    = -1;
        int effect_die_sides_3      = -1;
        int effect_amplitude_3      = -1;
        int effect_radius_index_3   = -1;
    };
    Cols pm_cols;

    // ---- FK lookup caches ----
    // Each maps the row's id to a small struct of resolved values. Loaded
    // lazily on first need (the tables are tiny: a few hundred rows each).
    struct CastTimeEntry { int32_t base_ms = 0; int32_t per_level = 0; int32_t min_ms = 0; };
    struct RangeEntry    { float min_yd = 0; float max_yd = 0; std::string name; };
    struct DurationEntry { int32_t base_ms = 0; int32_t per_level = 0; int32_t max_ms = 0; };
    struct RadiusEntry   { float radius = 0; float per_level = 0; float max = 0; };

    std::unordered_map<int64_t, CastTimeEntry> pm_cast_times;
    std::unordered_map<int64_t, RangeEntry>    pm_ranges;
    std::unordered_map<int64_t, DurationEntry> pm_durations;
    std::unordered_map<int64_t, RadiusEntry>   pm_radii;
    bool pm_fk_loaded = false;
    void EnsureFkTablesLoaded();

    // Resolvers — return formatted display strings; empty if id unknown.
    std::string ResolveCastTime(int64_t id) const;
    std::string ResolveRange(int64_t id) const;
    std::string ResolveDuration(int64_t id) const;
    std::string ResolveRadius(int64_t id) const;
    // Returns total duration in MS for description substitution.
    int32_t DurationMs(int64_t duration_index) const;

    // Description token resolver. Substitutes $s1/$m1/$M1/$o1/$d/$t1/$a1/$r/$n/$h
    // using values from the given row.
    std::string SubstituteDescription(const std::string& tpl, size_t row_idx);

    // Per-section drawing.
    void DrawHeaderSection(size_t row_idx, int64_t spell_id);
    void DrawIdentitySection(int64_t spell_id, size_t row_idx);
    void DrawDescriptionSection(int64_t spell_id, size_t row_idx);
    void DrawCostCastSection(int64_t spell_id, size_t row_idx);
    void DrawEffect1Section(int64_t spell_id, size_t row_idx);

    // Helper used by DrawDetail's per-row label/InputText editor pattern.
    // Returns true if a commit happened.
    bool DrawTextField(const char* label, int col_index,
                       int64_t spell_id, size_t row_idx,
                       DbcFieldType type);

    // Render a value-only display cell ("3.0 sec", "35 yd") — non-editable
    // because resolving back to an FK index is non-trivial; for editing the
    // raw index, expose the index field separately.
    void DrawResolvedField(const char* label, const std::string& value,
                           const char* edit_hint = nullptr);

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
