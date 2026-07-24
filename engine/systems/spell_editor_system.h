#ifndef MVE_SPELL_EDITOR_SYSTEM_H
#define MVE_SPELL_EDITOR_SYSTEM_H

#include "../db/db_connection.h"
#include "../resources/icon_cache.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace mve {

// Curated Spell.dbc editor - the friendly "spell card" view (vs. the generic
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

    // Convenience accessors over the cached row data - return defaults on miss.
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
        // Mana / power costs - Spell.dbc uses several columns; non-zero
        // entries are summed/combined at runtime by the WoW client.
        int mana_cost            = -1;
        int mana_cost_percentage = -1;
        int mana_cost_per_level  = -1;
        int mana_per_second      = -1;
        int mana_per_second_per_level = -1;
        int power_type           = -1;
        // Timing FKs - small lookup DBCs.
        int cast_time_index = -1;
        int range_index     = -1;
        int duration_index  = -1;
        int recovery_time   = -1;
        // Other identity / classification.
        int school_mask  = -1;
        int spell_family = -1;
        int max_targets  = -1;
        int proc_chance  = -1;
        // Effects (slot 1 - slot 2/3 added in next branch).
        int effect_1                = -1;
        int effect_base_points_1    = -1;
        int effect_die_sides_1      = -1;
        int effect_amplitude_1      = -1;  // tick period in ms for periodic effects
        int effect_radius_index_1   = -1;
        // Per-effect (we read 2/3 for description token substitution even though
        // we don't render their section yet - $s2, $o3 might be referenced).
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

        // Attributes - 8 bitmask columns total.
        int attributes_attr[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

        // Reagents - 8 slots of (item id, count).
        int reagent[8]       = { -1, -1, -1, -1, -1, -1, -1, -1 };
        int reagent_count[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

        // Cooldown / category
        int category = -1;

        // Cross-reference fields (FK to other spell IDs).
        int caster_aura_spell         = -1;
        int target_aura_spell         = -1;
        int exclude_caster_aura_spell = -1;
        int exclude_target_aura_spell = -1;
        int effect_trigger_spell[3]   = { -1, -1, -1 };

        // SpellFamily fields for the modifier graph.
        int spell_family_name           = -1;
        int spell_family_flags[3]       = { -1, -1, -1 };
        int effect_spell_class_mask_a[3] = { -1, -1, -1 };
        int effect_spell_class_mask_b[3] = { -1, -1, -1 };
        int effect_spell_class_mask_c[3] = { -1, -1, -1 };
    };
    Cols pm_cols;

    // ---- Cooldown category index ----
    // category_id -> list of row indices in pm_spells that share it. Built
    // once at load time so the "shares cooldown with" lookup is O(1).
    std::unordered_map<int64_t, std::vector<int>> pm_category_to_rows;

    // ---- Cross-reference indices (Branch Z) ----
    // Spell id -> row index in pm_spells. Used by JumpToSpell + reverse
    // lookups. Built at load time during CacheColumnIndices().
    std::unordered_map<int64_t, int> pm_id_to_row;

    // Reverse-reference kind: which field on a source spell points at our
    // target spell. We store enough kinds to render meaningful "Referenced
    // by" sub-headers.
    enum class RevRefKind {
        CasterAura,
        TargetAura,
        ExcludeCasterAura,
        ExcludeTargetAura,
        TriggerSpell1,
        TriggerSpell2,
        TriggerSpell3,
    };
    struct RevRef { int source_row; RevRefKind kind; };

    // Target spell id -> list of (source row, kind) entries.
    std::unordered_map<int64_t, std::vector<RevRef>> pm_reverse_refs;

    // Talent + GlyphProperties tables and the spell-id -> entry indices.
    DbConnection::Table pm_talents;
    DbConnection::Table pm_talent_tabs;
    DbConnection::Table pm_glyphs;
    struct TalentRef { int talent_row; int rank_index; };  // rank 0..8
    std::unordered_map<int64_t, std::vector<TalentRef>> pm_talent_refs;
    std::unordered_map<int64_t, std::vector<int64_t>> pm_glyph_refs;
    // tab id -> name_enus
    std::unordered_map<int64_t, std::string> pm_talent_tab_names;

    // Per-spell-open cache of "modifier spells that target me via family
    // class mask." Keyed by the spell id we currently have selected.
    std::unordered_map<int64_t, std::vector<std::pair<int, int>>>
        pm_family_modifier_cache;  // value = list of (source_row, effect_slot)

    // Builders called from EnsureLoaded() after CacheColumnIndices().
    void BuildSpellCrossRefs();
    void BuildTalentAndGlyphRefs();
    const std::vector<std::pair<int, int>>&
        GetFamilyModifiersFor(int64_t spell_id, size_t row_idx);

    // Navigation helpers.
    void JumpToSpell(int64_t spell_id);
    // Renders "<Name>  (#id)" as a clickable Selectable. On click, jumps to
    // that spell. Returns true if clicked. Unknown id renders just "#id" but
    // is still clickable as a no-op so the layout stays stable.
    bool SpellLink(int64_t spell_id);

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

    // Resolvers - return formatted display strings; empty if id unknown.
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
    // Generic effect renderer: slot = 0 / 1 / 2 corresponds to Effect 1/2/3.
    void DrawEffectSection(int slot, int64_t spell_id, size_t row_idx);
    void DrawAttributesSection(int64_t spell_id, size_t row_idx);
    void DrawReagentsSection(size_t row_idx);
    void DrawCooldownDetailsSection(size_t row_idx);
    void DrawConditionsLinksSection(int64_t spell_id, size_t row_idx);
    void DrawReferencedBySection(int64_t spell_id);
    void DrawTalentsSection(int64_t spell_id);
    void DrawGlyphsSection(int64_t spell_id);
    void DrawFamilyModifiersSection(int64_t spell_id, size_t row_idx);

    // Helper used by DrawDetail's per-row label/InputText editor pattern.
    // Returns true if a commit happened.
    bool DrawTextField(const char* label, int col_index,
                       int64_t spell_id, size_t row_idx,
                       DbcFieldType type);

    // Render a value-only display cell ("3.0 sec", "35 yd") - non-editable
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
