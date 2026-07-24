#include "spell_editor_system.h"
#include "dbc_naming.h"
#include "enum_registry.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace mve {

namespace {

// Find a column index by snake_case name. Returns -1 if missing.
int FindCol(const std::vector<std::string>& columns, const char* name) {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i] == name) return static_cast<int>(i);
    }
    return -1;
}

std::string ToLowerCopy(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

bool MatchesFilter(const std::string& haystack_lower, const std::string& filter_lower) {
    if (filter_lower.empty()) return true;
    return haystack_lower.find(filter_lower) != std::string::npos;
}

// Map SpellSchoolMask bit -> friendly name. Spell.dbc stores the school as
// a single-bit mask, so picking the first set bit suffices for display.
const char* SchoolNameFromMask(int64_t mask) {
    switch (mask) {
    case 1:   return "Physical";
    case 2:   return "Holy";
    case 4:   return "Fire";
    case 8:   return "Nature";
    case 16:  return "Frost";
    case 32:  return "Shadow";
    case 64:  return "Arcane";
    default:  return "(multi)";
    }
}

constexpr float kIconSize = 32.0f;
constexpr float kBigIconSize = 64.0f;

} // namespace

SpellEditorSystem::SpellEditorSystem(DbConnection& db, IconCache& icons)
    : pm_db{db}, pm_icons{icons} {}

void SpellEditorSystem::EnsureLoaded() {
    if (pm_loaded || !pm_load_error.empty()) return;
    if (!pm_db.IsConnected()) {
        pm_load_error = "Not connected to PostgreSQL.";
        return;
    }

    if (!pm_db.FetchTable("spell", pm_spells)) {
        pm_load_error = "Fetch spell failed: " + pm_db.LastError();
        return;
    }
    CacheColumnIndices();
    BuildSpellCrossRefs();
    BuildTalentAndGlyphRefs();

    // Pre-compute lowercase names for filter matching.
    pm_lowercase_names.clear();
    pm_lowercase_names.reserve(pm_spells.rows.size());
    for (const auto& row : pm_spells.rows) {
        if (pm_cols.name >= 0 &&
            pm_cols.name < static_cast<int>(row.values.size())) {
            pm_lowercase_names.push_back(ToLowerCopy(row.values[pm_cols.name]));
        } else {
            pm_lowercase_names.emplace_back();
        }
    }

    // Load SpellIcon table for icon-id -> path lookup.
    DbConnection::Table icons_tbl;
    if (pm_db.FetchTable("spellicon", icons_tbl)) {
        int id_col = FindCol(icons_tbl.columns, "id");
        int path_col = FindCol(icons_tbl.columns, "icon_path");
        if (id_col >= 0 && path_col >= 0) {
            for (const auto& r : icons_tbl.rows) {
                if (id_col < static_cast<int>(r.values.size()) &&
                    path_col < static_cast<int>(r.values.size())) {
                    int64_t id = std::strtoll(r.values[id_col].c_str(), nullptr, 10);
                    pm_icon_paths.emplace(id, r.values[path_col]);
                }
            }
        }
    }

    pm_loaded = true;
}

void SpellEditorSystem::CacheColumnIndices() {
    auto& c = pm_cols;
    auto& cols = pm_spells.columns;

    // Always route lookups through DbcColumnName so the lookup string is
    // produced by the same algorithm the importer uses. Hardcoding the
    // expected output is fragile because the algorithm treats all-caps
    // runs as multiple boundaries.
    auto col = [&](const char* schema_field) -> int {
        return FindCol(cols, DbcColumnName(schema_field).c_str());
    };

    c.id              = col("Id");
    c.name            = col("SpellName_enUS");
    c.rank            = col("Rank_enUS");
    c.description     = col("Description_enUS");
    c.icon_id         = col("SpellIconID");

    // Power cost columns
    c.mana_cost                  = col("ManaCost");
    c.mana_cost_percentage       = col("ManaCostPercentage");
    c.mana_cost_per_level        = col("ManaCostPerLevel");
    c.mana_per_second            = col("ManaPerSecond");
    c.mana_per_second_per_level  = col("ManaPerSecondPerLevel");
    c.power_type                 = col("PowerType");

    // Timing FKs
    c.cast_time_index = col("CastingTimeIndex");
    c.range_index     = col("RangeIndex");
    c.duration_index  = col("DurationIndex");
    c.recovery_time   = col("RecoveryTime");

    c.school_mask     = col("SchoolMask");
    c.spell_family    = col("SpellFamilyName");
    c.max_targets     = col("MaxAffectedTargets");
    c.proc_chance     = col("ProcChance");

    // Effect slot 1
    c.effect_1                = col("Effect1");
    c.effect_base_points_1    = col("EffectBasePoints1");
    c.effect_die_sides_1      = col("EffectDieSides1");
    c.effect_amplitude_1      = col("EffectAmplitude1");
    c.effect_radius_index_1   = col("EffectRadiusIndex1");

    // Effect slots 2 and 3 - read-only for now (we substitute $s2/$o3 etc.
    // into descriptions even though we don't yet have their own section).
    c.effect_2                = col("Effect2");
    c.effect_base_points_2    = col("EffectBasePoints2");
    c.effect_die_sides_2      = col("EffectDieSides2");
    c.effect_amplitude_2      = col("EffectAmplitude2");
    c.effect_radius_index_2   = col("EffectRadiusIndex2");
    c.effect_3                = col("Effect3");
    c.effect_base_points_3    = col("EffectBasePoints3");
    c.effect_die_sides_3      = col("EffectDieSides3");
    c.effect_amplitude_3      = col("EffectAmplitude3");
    c.effect_radius_index_3   = col("EffectRadiusIndex3");

    // Attribute bitmasks - 8 columns.
    c.attributes_attr[0] = col("Attributes");
    c.attributes_attr[1] = col("AttributesEx");
    c.attributes_attr[2] = col("AttributesEx2");
    c.attributes_attr[3] = col("AttributesEx3");
    c.attributes_attr[4] = col("AttributesEx4");
    c.attributes_attr[5] = col("AttributesEx5");
    c.attributes_attr[6] = col("AttributesEx6");
    c.attributes_attr[7] = col("AttributesEx7");

    // Reagents - 8 (item id, count) pairs.
    c.reagent[0]       = col("Reagent1");
    c.reagent[1]       = col("Reagent2");
    c.reagent[2]       = col("Reagent3");
    c.reagent[3]       = col("Reagent4");
    c.reagent[4]       = col("Reagent5");
    c.reagent[5]       = col("Reagent6");
    c.reagent[6]       = col("Reagent7");
    c.reagent[7]       = col("Reagent8");
    c.reagent_count[0] = col("ReagentCount1");
    c.reagent_count[1] = col("ReagentCount2");
    c.reagent_count[2] = col("ReagentCount3");
    c.reagent_count[3] = col("ReagentCount4");
    c.reagent_count[4] = col("ReagentCount5");
    c.reagent_count[5] = col("ReagentCount6");
    c.reagent_count[6] = col("ReagentCount7");
    c.reagent_count[7] = col("ReagentCount8");

    c.category = col("Category");

    // Cross-reference fields (Branch Z).
    c.caster_aura_spell         = col("CasterAuraSpell");
    c.target_aura_spell         = col("TargetAuraSpell");
    c.exclude_caster_aura_spell = col("ExcludeCasterAuraSpell");
    c.exclude_target_aura_spell = col("ExcludeTargetAuraSpell");
    c.effect_trigger_spell[0]   = col("EffectTriggerSpell1");
    c.effect_trigger_spell[1]   = col("EffectTriggerSpell2");
    c.effect_trigger_spell[2]   = col("EffectTriggerSpell3");

    c.spell_family_name           = col("SpellFamilyName");
    c.spell_family_flags[0]       = col("SpellFamilyFlags1");
    c.spell_family_flags[1]       = col("SpellFamilyFlags2");
    c.spell_family_flags[2]       = col("SpellFamilyFlags3");
    c.effect_spell_class_mask_a[0] = col("EffectSpellClassMaskA1");
    c.effect_spell_class_mask_a[1] = col("EffectSpellClassMaskA2");
    c.effect_spell_class_mask_a[2] = col("EffectSpellClassMaskA3");
    c.effect_spell_class_mask_b[0] = col("EffectSpellClassMaskB1");
    c.effect_spell_class_mask_b[1] = col("EffectSpellClassMaskB2");
    c.effect_spell_class_mask_b[2] = col("EffectSpellClassMaskB3");
    c.effect_spell_class_mask_c[0] = col("EffectSpellClassMaskC1");
    c.effect_spell_class_mask_c[1] = col("EffectSpellClassMaskC2");
    c.effect_spell_class_mask_c[2] = col("EffectSpellClassMaskC3");

    // Build category -> rows reverse-index now that column indices are known.
    pm_category_to_rows.clear();
    if (c.category >= 0) {
        for (size_t i = 0; i < pm_spells.rows.size(); i++) {
            int64_t cat = GetCellInt(i, c.category);
            if (cat > 0) {
                pm_category_to_rows[cat].push_back(static_cast<int>(i));
            }
        }
    }
}

// ---- Cross-reference indices (Branch Z) -----------------------------------
//
// Builds three indices at load time:
//   pm_id_to_row    spell id -> row idx (for JumpToSpell + lookups)
//   pm_reverse_refs target spell id -> [(source row, ref kind)]
//
// One O(N) pass over the spell table after columns are cached. Family
// modifiers are computed lazily on demand instead.
void SpellEditorSystem::BuildSpellCrossRefs() {
    pm_id_to_row.clear();
    pm_reverse_refs.clear();

    // First pass: id -> row.
    if (pm_cols.id < 0) return;
    pm_id_to_row.reserve(pm_spells.rows.size());
    for (size_t i = 0; i < pm_spells.rows.size(); i++) {
        int64_t id = GetCellInt(i, pm_cols.id);
        if (id > 0) pm_id_to_row[id] = static_cast<int>(i);
    }

    // Second pass: walk every row, record each non-zero cross-ref as a
    // reverse entry on the target id.
    auto record = [&](size_t src_row, int col, RevRefKind kind) {
        if (col < 0) return;
        int64_t tgt = GetCellInt(src_row, col);
        if (tgt <= 0) return;
        pm_reverse_refs[tgt].push_back({ static_cast<int>(src_row), kind });
    };
    for (size_t i = 0; i < pm_spells.rows.size(); i++) {
        record(i, pm_cols.caster_aura_spell,         RevRefKind::CasterAura);
        record(i, pm_cols.target_aura_spell,         RevRefKind::TargetAura);
        record(i, pm_cols.exclude_caster_aura_spell, RevRefKind::ExcludeCasterAura);
        record(i, pm_cols.exclude_target_aura_spell, RevRefKind::ExcludeTargetAura);
        record(i, pm_cols.effect_trigger_spell[0],   RevRefKind::TriggerSpell1);
        record(i, pm_cols.effect_trigger_spell[1],   RevRefKind::TriggerSpell2);
        record(i, pm_cols.effect_trigger_spell[2],   RevRefKind::TriggerSpell3);
    }
}

void SpellEditorSystem::BuildTalentAndGlyphRefs() {
    pm_talent_refs.clear();
    pm_glyph_refs.clear();
    pm_talent_tab_names.clear();

    // ---- Talent ----------------------------------------------------------
    if (pm_db.FetchTable("talent", pm_talents)) {
        int id_col  = FindCol(pm_talents.columns, DbcColumnName("Id").c_str());
        int tab_col = FindCol(pm_talents.columns, DbcColumnName("TabID").c_str());
        int rank_cols[9];
        for (int r = 0; r < 9; r++) {
            char fname[16];
            std::snprintf(fname, sizeof(fname), "SpellRank%d", r + 1);
            rank_cols[r] = FindCol(pm_talents.columns, DbcColumnName(fname).c_str());
        }
        (void)id_col; (void)tab_col;  // referenced via row access pattern

        for (size_t i = 0; i < pm_talents.rows.size(); i++) {
            for (int r = 0; r < 9; r++) {
                if (rank_cols[r] < 0) continue;
                const auto& v = pm_talents.rows[i].values;
                if (rank_cols[r] >= static_cast<int>(v.size())) continue;
                int64_t spell_id = std::strtoll(v[rank_cols[r]].c_str(), nullptr, 10);
                if (spell_id > 0) {
                    pm_talent_refs[spell_id].push_back({ static_cast<int>(i), r });
                }
            }
        }
    }

    // ---- TalentTab (for friendly tab names) ------------------------------
    if (pm_db.FetchTable("talenttab", pm_talent_tabs)) {
        int id_col   = FindCol(pm_talent_tabs.columns, DbcColumnName("Id").c_str());
        int name_col = FindCol(pm_talent_tabs.columns,
                               DbcColumnName("Name_enUS").c_str());
        for (const auto& r : pm_talent_tabs.rows) {
            if (id_col < 0 || name_col < 0) break;
            if (id_col >= static_cast<int>(r.values.size()) ||
                name_col >= static_cast<int>(r.values.size())) continue;
            int64_t tab_id = std::strtoll(r.values[id_col].c_str(), nullptr, 10);
            pm_talent_tab_names[tab_id] = r.values[name_col];
        }
    }

    // ---- GlyphProperties --------------------------------------------------
    if (pm_db.FetchTable("glyphproperties", pm_glyphs)) {
        int id_col    = FindCol(pm_glyphs.columns, DbcColumnName("Id").c_str());
        int spell_col = FindCol(pm_glyphs.columns, DbcColumnName("SpellID").c_str());
        for (const auto& r : pm_glyphs.rows) {
            if (id_col < 0 || spell_col < 0) break;
            if (id_col >= static_cast<int>(r.values.size()) ||
                spell_col >= static_cast<int>(r.values.size())) continue;
            int64_t glyph_id = std::strtoll(r.values[id_col].c_str(), nullptr, 10);
            int64_t spell_id = std::strtoll(r.values[spell_col].c_str(), nullptr, 10);
            if (spell_id > 0) {
                pm_glyph_refs[spell_id].push_back(glyph_id);
            }
        }
    }
}

const std::vector<std::pair<int, int>>&
SpellEditorSystem::GetFamilyModifiersFor(int64_t spell_id, size_t row_idx) {
    auto cached = pm_family_modifier_cache.find(spell_id);
    if (cached != pm_family_modifier_cache.end()) return cached->second;

    auto& out = pm_family_modifier_cache[spell_id];

    int64_t my_family = GetCellInt(row_idx, pm_cols.spell_family_name);
    if (my_family == 0) return out;

    uint32_t my_flags[3] = {
        static_cast<uint32_t>(GetCellInt(row_idx, pm_cols.spell_family_flags[0])),
        static_cast<uint32_t>(GetCellInt(row_idx, pm_cols.spell_family_flags[1])),
        static_cast<uint32_t>(GetCellInt(row_idx, pm_cols.spell_family_flags[2])),
    };
    if ((my_flags[0] | my_flags[1] | my_flags[2]) == 0) return out;

    // Scan all spells. Filter by family first, then check mask overlap per effect.
    // 49k iterations + 9 quick checks each is ~500us. Cached after first call.
    for (size_t i = 0; i < pm_spells.rows.size(); i++) {
        if (static_cast<int>(i) == static_cast<int>(row_idx)) continue;
        int64_t fam = GetCellInt(i, pm_cols.spell_family_name);
        if (fam != my_family) continue;

        for (int slot = 0; slot < 3; slot++) {
            uint32_t a = static_cast<uint32_t>(
                GetCellInt(i, pm_cols.effect_spell_class_mask_a[slot]));
            uint32_t b = static_cast<uint32_t>(
                GetCellInt(i, pm_cols.effect_spell_class_mask_b[slot]));
            uint32_t cmask = static_cast<uint32_t>(
                GetCellInt(i, pm_cols.effect_spell_class_mask_c[slot]));
            if ((a & my_flags[0]) | (b & my_flags[1]) | (cmask & my_flags[2])) {
                out.push_back({ static_cast<int>(i), slot });
                break;  // one match per source spell is enough for display
            }
        }
        if (out.size() >= 200) break;  // cap to keep UI usable
    }
    return out;
}

// ---- Navigation helpers ---------------------------------------------------

void SpellEditorSystem::JumpToSpell(int64_t spell_id) {
    auto it = pm_id_to_row.find(spell_id);
    if (it == pm_id_to_row.end()) return;
    pm_selected_row = it->second;
    pm_edit_buffers.clear();
    pm_last_error.clear();
}

bool SpellEditorSystem::SpellLink(int64_t spell_id) {
    auto it = pm_id_to_row.find(spell_id);
    char label[160];
    if (it != pm_id_to_row.end()) {
        std::string name = GetCell(static_cast<size_t>(it->second), pm_cols.name);
        std::snprintf(label, sizeof(label), "%s  (#%lld)",
                      name.empty() ? "(unnamed)" : name.c_str(),
                      static_cast<long long>(spell_id));
    } else {
        std::snprintf(label, sizeof(label), "#%lld  (unknown)",
                      static_cast<long long>(spell_id));
    }
    ImGui::PushID(static_cast<int>(spell_id));
    bool clicked = ImGui::Selectable(label, false, ImGuiSelectableFlags_None);
    ImGui::PopID();
    if (clicked) JumpToSpell(spell_id);
    return clicked;
}

// ---- FK lookup tables ------------------------------------------------------
//
// Lazy-loaded once. Each of these tables is small (a few hundred rows)
// so a full FetchTable is cheap.
void SpellEditorSystem::EnsureFkTablesLoaded() {
    if (pm_fk_loaded || !pm_db.IsConnected()) return;
    pm_fk_loaded = true;

    auto fetch = [&](const char* table_name, auto on_row) {
        DbConnection::Table t;
        if (!pm_db.FetchTable(table_name, t)) return;
        int id_col = FindCol(t.columns, "id");
        if (id_col < 0) return;
        for (auto& r : t.rows) {
            if (id_col >= static_cast<int>(r.values.size())) continue;
            int64_t id = std::strtoll(r.values[id_col].c_str(), nullptr, 10);
            on_row(id, r, t.columns);
        }
    };

    // SpellCastTimes: Base / PerLevel / Minimum (all milliseconds).
    fetch("spellcasttimes", [&](int64_t id, const DbConnection::Row& r,
                                const std::vector<std::string>& cols) {
        int b = FindCol(cols, DbcColumnName("Base").c_str());
        int p = FindCol(cols, DbcColumnName("PerLevel").c_str());
        int m = FindCol(cols, DbcColumnName("Minimum").c_str());
        CastTimeEntry e;
        if (b >= 0) e.base_ms   = std::atoi(r.values[b].c_str());
        if (p >= 0) e.per_level = std::atoi(r.values[p].c_str());
        if (m >= 0) e.min_ms    = std::atoi(r.values[m].c_str());
        pm_cast_times[id] = e;
    });

    // SpellRange: MinRange / MaxRange / Name (we use the english name).
    fetch("spellrange", [&](int64_t id, const DbConnection::Row& r,
                            const std::vector<std::string>& cols) {
        int mn = FindCol(cols, DbcColumnName("MinRange").c_str());
        int mx = FindCol(cols, DbcColumnName("MaxRange").c_str());
        int nm = FindCol(cols, DbcColumnName("Name_enUS").c_str());
        RangeEntry e;
        if (mn >= 0) e.min_yd = std::strtof(r.values[mn].c_str(), nullptr);
        if (mx >= 0) e.max_yd = std::strtof(r.values[mx].c_str(), nullptr);
        if (nm >= 0) e.name = r.values[nm];
        pm_ranges[id] = std::move(e);
    });

    // SpellDuration: Duration / MaxDuration in MS, PerLevel scaling.
    fetch("spellduration", [&](int64_t id, const DbConnection::Row& r,
                                const std::vector<std::string>& cols) {
        int b = FindCol(cols, DbcColumnName("Duration").c_str());
        int p = FindCol(cols, DbcColumnName("DurationPerLevel").c_str());
        int m = FindCol(cols, DbcColumnName("MaxDuration").c_str());
        DurationEntry e;
        if (b >= 0) e.base_ms   = std::atoi(r.values[b].c_str());
        if (p >= 0) e.per_level = std::atoi(r.values[p].c_str());
        if (m >= 0) e.max_ms    = std::atoi(r.values[m].c_str());
        pm_durations[id] = e;
    });

    // SpellRadius: Radius / RadiusPerLevel / MaxRadius (yards).
    fetch("spellradius", [&](int64_t id, const DbConnection::Row& r,
                              const std::vector<std::string>& cols) {
        int b = FindCol(cols, DbcColumnName("Radius").c_str());
        int p = FindCol(cols, DbcColumnName("RadiusPerLevel").c_str());
        int m = FindCol(cols, DbcColumnName("MaxRadius").c_str());
        RadiusEntry e;
        if (b >= 0) e.radius   = std::strtof(r.values[b].c_str(), nullptr);
        if (p >= 0) e.per_level = std::strtof(r.values[p].c_str(), nullptr);
        if (m >= 0) e.max      = std::strtof(r.values[m].c_str(), nullptr);
        pm_radii[id] = e;
    });
}

std::string SpellEditorSystem::ResolveCastTime(int64_t id) const {
    auto it = pm_cast_times.find(id);
    if (it == pm_cast_times.end() || it->second.base_ms == 0) return {};
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f sec", it->second.base_ms / 1000.0f);
    return buf;
}

std::string SpellEditorSystem::ResolveRange(int64_t id) const {
    auto it = pm_ranges.find(id);
    if (it == pm_ranges.end()) return {};
    const auto& r = it->second;
    char buf[64];
    if (r.min_yd > 0) std::snprintf(buf, sizeof(buf), "%.0f-%.0f yd", r.min_yd, r.max_yd);
    else              std::snprintf(buf, sizeof(buf), "%.0f yd", r.max_yd);
    return buf;
}

std::string SpellEditorSystem::ResolveDuration(int64_t id) const {
    auto it = pm_durations.find(id);
    if (it == pm_durations.end() || it->second.base_ms == 0) return {};
    char buf[32];
    float sec = it->second.base_ms / 1000.0f;
    if (sec >= 60.0f) std::snprintf(buf, sizeof(buf), "%.1f min", sec / 60.0f);
    else              std::snprintf(buf, sizeof(buf), "%.0f sec", sec);
    return buf;
}

std::string SpellEditorSystem::ResolveRadius(int64_t id) const {
    auto it = pm_radii.find(id);
    if (it == pm_radii.end() || it->second.radius == 0) return {};
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.0f yd", it->second.radius);
    return buf;
}

int32_t SpellEditorSystem::DurationMs(int64_t duration_index) const {
    auto it = pm_durations.find(duration_index);
    return (it == pm_durations.end()) ? 0 : it->second.base_ms;
}

// ---- Description token substitution -----------------------------------------
//
// Walks the template and substitutes a handful of WoW description tokens.
// We don't try to be fully faithful to the WoW client - the goal is "show
// approximate in-game-readable values so the editor isn't lying."
//
// Supported tokens (case-insensitive index):
//   $s1/$s2/$s3   effect points (range "X to Y" if random, else flat value)
//   $m1/$m2/$m3   min effect (BasePoints + 1)
//   $M1/$M2/$M3   max effect (BasePoints + DieSides)
//   $o1/$o2/$o3   over-time total: tick_value * (duration / period)
//   $t1/$t2/$t3   tick period (EffectAmplitude / 1000 seconds)
//   $a1/$a2/$a3   radius (resolved via EffectRadiusIndex)
//   $d            duration in seconds (resolved via DurationIndex)
//   $r            range max yards
//   $n            MaxAffectedTargets
//   $h            ProcChance
//   $$            literal $
std::string SpellEditorSystem::SubstituteDescription(const std::string& tpl,
                                                     size_t row_idx) {
    std::string out;
    out.reserve(tpl.size() + 32);

    int eff_cols[3]   = { pm_cols.effect_1,
                          pm_cols.effect_2,
                          pm_cols.effect_3 };
    int base_cols[3]  = { pm_cols.effect_base_points_1,
                          pm_cols.effect_base_points_2,
                          pm_cols.effect_base_points_3 };
    int die_cols[3]   = { pm_cols.effect_die_sides_1,
                          pm_cols.effect_die_sides_2,
                          pm_cols.effect_die_sides_3 };
    int amp_cols[3]   = { pm_cols.effect_amplitude_1,
                          pm_cols.effect_amplitude_2,
                          pm_cols.effect_amplitude_3 };
    int rad_cols[3]   = { pm_cols.effect_radius_index_1,
                          pm_cols.effect_radius_index_2,
                          pm_cols.effect_radius_index_3 };

    auto base_for = [&](int slot) -> int64_t {
        return GetCellInt(row_idx, base_cols[slot]);
    };
    auto die_for = [&](int slot) -> int64_t {
        return GetCellInt(row_idx, die_cols[slot]);
    };
    auto amp_for = [&](int slot) -> int64_t {
        return GetCellInt(row_idx, amp_cols[slot]);
    };

    auto effect_range = [&](int slot) {
        int64_t b = base_for(slot);
        int64_t d = die_for(slot);
        if (d <= 0) return std::to_string(b + 1);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%lld to %lld",
                      static_cast<long long>(b + 1),
                      static_cast<long long>(b + d));
        return std::string{buf};
    };

    int32_t dur_ms = DurationMs(GetCellInt(row_idx, pm_cols.duration_index));
    int64_t max_targets = GetCellInt(row_idx, pm_cols.max_targets);
    int64_t proc_chance = GetCellInt(row_idx, pm_cols.proc_chance);
    int64_t range_idx   = GetCellInt(row_idx, pm_cols.range_index);

    for (size_t i = 0; i < tpl.size(); ) {
        char c = tpl[i];
        if (c != '$') { out += c; ++i; continue; }

        if (i + 1 >= tpl.size()) { out += c; ++i; continue; }

        char tk = tpl[i + 1];
        if (tk == '$') { out += '$'; i += 2; continue; }

        // Single-character tokens
        if (tk == 'd') {
            if (dur_ms > 0) {
                float sec = dur_ms / 1000.0f;
                char buf[32];
                if (sec >= 60.0f) std::snprintf(buf, sizeof(buf), "%.0f min", sec / 60.0f);
                else              std::snprintf(buf, sizeof(buf), "%.0f sec", sec);
                out += buf;
            } else {
                out += "$d";
            }
            i += 2; continue;
        }
        if (tk == 'r') {
            std::string r = ResolveRange(range_idx);
            out += r.empty() ? std::string{"$r"} : r;
            i += 2; continue;
        }
        if (tk == 'n') { out += std::to_string(max_targets); i += 2; continue; }
        if (tk == 'h') { out += std::to_string(proc_chance); i += 2; continue; }

        // Two-character tokens: <letter><digit>
        if (i + 2 < tpl.size()) {
            char ix = tpl[i + 2];
            if (ix >= '1' && ix <= '3') {
                int slot = ix - '1';
                if      (tk == 's') { out += effect_range(slot); i += 3; continue; }
                else if (tk == 'm') { out += std::to_string(base_for(slot) + 1); i += 3; continue; }
                else if (tk == 'M') { out += std::to_string(base_for(slot) + die_for(slot)); i += 3; continue; }
                else if (tk == 'o') {
                    // Over-time total = (BasePoints+1) * ticks
                    int64_t period_ms = amp_for(slot);
                    if (period_ms > 0 && dur_ms > 0) {
                        int64_t ticks = dur_ms / period_ms;
                        int64_t per_tick = base_for(slot) + 1;
                        out += std::to_string(per_tick * ticks);
                    } else {
                        out += std::to_string(base_for(slot) + 1);
                    }
                    i += 3; continue;
                }
                else if (tk == 't') {
                    int64_t period_ms = amp_for(slot);
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "%.0f", period_ms / 1000.0f);
                    out += buf;
                    i += 3; continue;
                }
                else if (tk == 'a') {
                    int64_t ridx = GetCellInt(row_idx, rad_cols[slot]);
                    std::string r = ResolveRadius(ridx);
                    out += r.empty() ? std::string{"$a"} + ix : r;
                    i += 3; continue;
                }
            }
        }

        // Unknown token - pass through verbatim so the user can see it.
        out += c; ++i;
    }
    return out;
}

std::string SpellEditorSystem::GetCell(size_t row_index, int col_index) const {
    if (col_index < 0 || row_index >= pm_spells.rows.size()) return {};
    const auto& row = pm_spells.rows[row_index];
    if (col_index >= static_cast<int>(row.values.size())) return {};
    return row.values[col_index];
}

int64_t SpellEditorSystem::GetCellInt(size_t row_index, int col_index) const {
    std::string s = GetCell(row_index, col_index);
    if (s.empty()) return 0;
    return std::strtoll(s.c_str(), nullptr, 10);
}

ImTextureID SpellEditorSystem::IconFor(size_t row_index) {
    if (pm_cols.icon_id < 0) return ImTextureID_Invalid;
    int64_t icon_id = GetCellInt(row_index, pm_cols.icon_id);
    if (icon_id == 0) return ImTextureID_Invalid;
    auto it = pm_icon_paths.find(icon_id);
    if (it == pm_icon_paths.end()) return ImTextureID_Invalid;
    return pm_icons.Get(it->second);
}

void SpellEditorSystem::Update() {
    // Give the window a usable initial size so a fresh imgui.ini (or one
    // where the user dragged this window off-screen) shows something
    // discoverable. ImGui_Cond_FirstUseEver means user resizes win after.
    ImGui::SetNextWindowSize(ImVec2(900, 640), ImGuiCond_FirstUseEver);
    ImGui::Begin("Spell Editor");

    EnsureLoaded();
    if (!pm_load_error.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", pm_load_error.c_str());
        ImGui::End();
        return;
    }
    if (!pm_loaded) {
        ImGui::TextDisabled("Loading...");
        ImGui::End();
        return;
    }

    // Two-pane layout. Left ~320px for the finder, right gets the rest.
    constexpr float kFinderWidth = 360.0f;
    ImGui::BeginChild("##finder", ImVec2(kFinderWidth, 0),
                      ImGuiChildFlags_Borders);
    DrawFinder();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##detail", ImVec2(0, 0), ImGuiChildFlags_Borders);
    DrawDetail();
    ImGui::EndChild();

    ImGui::End();
}

void SpellEditorSystem::DrawFinder() {
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##filter",
                             "Search by name (e.g. fireball)",
                             pm_filter, sizeof(pm_filter));
    ImGui::TextDisabled("%zu spells loaded", pm_spells.rows.size());
    ImGui::Separator();

    std::string filter_lower = ToLowerCopy(pm_filter);

    // Clipper handles the 50k-row case smoothly. But the clipper assumes a
    // contiguous index range - with filtering active, we need to build a
    // visible-index list first when there's a filter, then clip that list.
    std::vector<int> visible_indices;
    if (filter_lower.empty()) {
        visible_indices.resize(pm_spells.rows.size());
        for (size_t i = 0; i < visible_indices.size(); i++) {
            visible_indices[i] = static_cast<int>(i);
        }
    } else {
        visible_indices.reserve(256);
        for (size_t i = 0; i < pm_lowercase_names.size(); i++) {
            if (MatchesFilter(pm_lowercase_names[i], filter_lower)) {
                visible_indices.push_back(static_cast<int>(i));
                if (visible_indices.size() >= 2000) break;  // cap for sanity
            }
        }
        ImGui::TextDisabled("%zu match", visible_indices.size());
    }

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(visible_indices.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            int row_index = visible_indices[i];
            int64_t id = GetCellInt(row_index, pm_cols.id);
            std::string name = GetCell(row_index, pm_cols.name);
            std::string rank = GetCell(row_index, pm_cols.rank);

            ImGui::PushID(row_index);

            // Icon thumbnail on the left, name + rank on the right.
            ImTextureID tex = IconFor(row_index);
            if (tex != ImTextureID_Invalid) {
                ImGui::Image(tex, ImVec2(kIconSize, kIconSize));
                ImGui::SameLine();
            } else {
                // Reserve the icon slot so all rows line up.
                ImGui::Dummy(ImVec2(kIconSize, kIconSize));
                ImGui::SameLine();
            }

            ImGui::BeginGroup();
            bool selected = (pm_selected_row == row_index);
            char label[256];
            if (!rank.empty()) {
                std::snprintf(label, sizeof(label), "%s  (%s)##%d",
                              name.c_str(), rank.c_str(), row_index);
            } else {
                std::snprintf(label, sizeof(label), "%s##%d",
                              name.c_str(), row_index);
            }
            if (ImGui::Selectable(label, selected,
                                  ImGuiSelectableFlags_None,
                                  ImVec2(0, kIconSize))) {
                pm_selected_row = row_index;
                pm_edit_buffers.clear();
                pm_last_error.clear();
            }
            ImGui::TextDisabled("  #%lld", static_cast<long long>(id));
            ImGui::EndGroup();

            ImGui::PopID();
        }
    }
}

void SpellEditorSystem::DrawDetail() {
    if (pm_selected_row < 0 || pm_selected_row >= static_cast<int>(pm_spells.rows.size())) {
        ImGui::TextDisabled("Select a spell from the list on the left.");
        return;
    }

    EnsureFkTablesLoaded();

    size_t row_idx = static_cast<size_t>(pm_selected_row);
    int64_t spell_id = GetCellInt(row_idx, pm_cols.id);

    // Buffers are populated lazily inside DrawTextField. The description
    // is special-cased because its block runs inline in DrawDescriptionSection
    // rather than through DrawTextField.
    if (pm_cols.description >= 0 &&
        pm_edit_buffers.find(pm_cols.description) == pm_edit_buffers.end()) {
        pm_edit_buffers[pm_cols.description] = GetCell(row_idx, pm_cols.description);
    }

    DrawHeaderSection(row_idx, spell_id);
    ImGui::Separator();

    if (!pm_last_error.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", pm_last_error.c_str());
        ImGui::Separator();
    }

    DrawIdentitySection(spell_id, row_idx);
    DrawDescriptionSection(spell_id, row_idx);
    DrawCostCastSection(spell_id, row_idx);
    DrawConditionsLinksSection(spell_id, row_idx);
    DrawEffectSection(0, spell_id, row_idx);
    DrawEffectSection(1, spell_id, row_idx);
    DrawEffectSection(2, spell_id, row_idx);
    DrawAttributesSection(spell_id, row_idx);
    DrawReagentsSection(row_idx);
    DrawCooldownDetailsSection(row_idx);
    DrawReferencedBySection(spell_id);
    DrawTalentsSection(spell_id);
    DrawGlyphsSection(spell_id);
    DrawFamilyModifiersSection(spell_id, row_idx);
}

// ---- Per-section drawing ----------------------------------------------------

void SpellEditorSystem::DrawHeaderSection(size_t row_idx, int64_t spell_id) {
    ImTextureID tex = IconFor(row_idx);
    if (tex != ImTextureID_Invalid) {
        ImGui::Image(tex, ImVec2(kBigIconSize, kBigIconSize));
    } else {
        ImGui::Dummy(ImVec2(kBigIconSize, kBigIconSize));
    }
    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("%s", GetCell(row_idx, pm_cols.name).c_str());
    std::string rank = GetCell(row_idx, pm_cols.rank);
    if (!rank.empty()) ImGui::TextDisabled("%s", rank.c_str());

    int64_t mask = GetCellInt(row_idx, pm_cols.school_mask);
    ImGui::TextDisabled("School: %s    #%lld",
                        SchoolNameFromMask(mask),
                        static_cast<long long>(spell_id));
    ImGui::EndGroup();
}

bool SpellEditorSystem::DrawTextField(const char* label, int col_index,
                                      int64_t spell_id, size_t row_idx,
                                      DbcFieldType type) {
    if (col_index < 0) return false;
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);

    // Lazy-init buffer from the row's current value. Robust to columns we
    // forgot to enumerate in the upfront ensure_buf list - any field hooked
    // up to DrawTextField gets a populated buffer the first time it's drawn.
    auto buf_it = pm_edit_buffers.find(col_index);
    if (buf_it == pm_edit_buffers.end()) {
        pm_edit_buffers[col_index] = GetCell(row_idx, col_index);
    }
    std::string& buf = pm_edit_buffers[col_index];
    if (buf.capacity() < 256) buf.reserve(256);
    ImGui::PushID(col_index);
    ImGui::InputText(
        "##v", buf.data(), buf.capacity(),
        ImGuiInputTextFlags_CallbackResize,
        [](ImGuiInputTextCallbackData* d) -> int {
            auto* s = static_cast<std::string*>(d->UserData);
            if (d->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                s->resize(d->BufTextLen);
                d->Buf = s->data();
            }
            return 0;
        }, &buf);
    bool committed = ImGui::IsItemDeactivatedAfterEdit();
    ImGui::PopID();

    if (committed) {
        std::string val = buf.c_str();
        Commit(spell_id, static_cast<size_t>(pm_selected_row),
               pm_spells.columns[col_index], val, type);
    }
    return committed;
}

void SpellEditorSystem::DrawResolvedField(const char* label, const std::string& value,
                                          const char* edit_hint) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", label);
    ImGui::TableSetColumnIndex(1);
    ImGui::AlignTextToFramePadding();
    if (value.empty()) {
        ImGui::TextDisabled("(none)");
    } else {
        ImGui::TextUnformatted(value.c_str());
    }
    if (edit_hint && *edit_hint) {
        ImGui::SameLine();
        ImGui::TextDisabled("  %s", edit_hint);
    }
}

void SpellEditorSystem::DrawIdentitySection(int64_t spell_id, size_t row_idx) {
    if (!ImGui::CollapsingHeader("Identity", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (!ImGui::BeginTable("##identity", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 180);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);
    DrawTextField("Name", pm_cols.name, spell_id, row_idx, DbcFieldType::String);
    DrawTextField("Rank", pm_cols.rank, spell_id, row_idx, DbcFieldType::String);
    ImGui::EndTable();
}

void SpellEditorSystem::DrawDescriptionSection(int64_t spell_id, size_t row_idx) {
    if (pm_cols.description < 0) return;
    if (!ImGui::CollapsingHeader("Description", ImGuiTreeNodeFlags_DefaultOpen)) return;

    std::string& buf = pm_edit_buffers[pm_cols.description];
    if (buf.capacity() < 2048) buf.reserve(2048);

    if (!ImGui::BeginTable("##descsplit", 2,
                           ImGuiTableFlags_SizingStretchSame |
                           ImGuiTableFlags_BordersInnerV)) return;
    ImGui::TableSetupColumn("##template", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("##preview",  ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("Template (editable)");
    ImGui::PushID(pm_cols.description);
    ImGui::InputTextMultiline(
        "##desc_tpl", buf.data(), buf.capacity(),
        ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5),
        ImGuiInputTextFlags_CallbackResize,
        [](ImGuiInputTextCallbackData* d) -> int {
            auto* s = static_cast<std::string*>(d->UserData);
            if (d->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                s->resize(d->BufTextLen);
                d->Buf = s->data();
            }
            return 0;
        }, &buf);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        Commit(spell_id, row_idx, pm_spells.columns[pm_cols.description],
               buf.c_str(), DbcFieldType::String);
    }
    ImGui::PopID();

    ImGui::TableSetColumnIndex(1);
    ImGui::TextDisabled("Preview (tokens resolved)");
    std::string preview = SubstituteDescription(buf.c_str(), row_idx);
    ImGui::BeginChild("##descprev",
                      ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5),
                      ImGuiChildFlags_Borders);
    ImGui::TextWrapped("%s", preview.c_str());
    ImGui::EndChild();

    ImGui::EndTable();

    ImGui::TextDisabled(
        "Tokens: $s1-$s3 effect range,  $m/$M min/max,  $o over-time total,  "
        "$d duration,  $t tick,  $a radius,  $r range,  $n max targets,  $h proc%%");
}

void SpellEditorSystem::DrawCostCastSection(int64_t spell_id, size_t row_idx) {
    if (!ImGui::CollapsingHeader("Cost & Cast", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (!ImGui::BeginTable("##costcast", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 200);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    // ---- Mana / power cost composite ----
    DrawTextField("Mana cost (flat)",      pm_cols.mana_cost,
                  spell_id, row_idx, DbcFieldType::UInt32);
    DrawTextField("Mana cost (% of base)", pm_cols.mana_cost_percentage,
                  spell_id, row_idx, DbcFieldType::UInt32);
    DrawTextField("Mana cost per level",   pm_cols.mana_cost_per_level,
                  spell_id, row_idx, DbcFieldType::UInt32);
    DrawTextField("Mana per second",       pm_cols.mana_per_second,
                  spell_id, row_idx, DbcFieldType::UInt32);
    DrawTextField("Mana per sec / level",  pm_cols.mana_per_second_per_level,
                  spell_id, row_idx, DbcFieldType::UInt32);

    int64_t mc       = GetCellInt(row_idx, pm_cols.mana_cost);
    int64_t mc_pct   = GetCellInt(row_idx, pm_cols.mana_cost_percentage);
    int64_t mc_pl    = GetCellInt(row_idx, pm_cols.mana_cost_per_level);
    int64_t mps      = GetCellInt(row_idx, pm_cols.mana_per_second);
    {
        std::ostringstream summary;
        bool first = true;
        auto add = [&](const std::string& s) {
            if (!first) summary << " + ";
            summary << s; first = false;
        };
        if (mc > 0)     add(std::to_string(mc) + " mana");
        if (mc_pct > 0) add(std::to_string(mc_pct) + "% of base mana");
        if (mc_pl > 0)  add(std::to_string(mc_pl) + " mana / level");
        if (mps > 0)    add(std::to_string(mps) + " mana / sec");
        if (first) summary << "free (no mana cost)";
        DrawResolvedField("Effective cost", summary.str());
    }

    // ---- Timing ----
    int64_t cti = GetCellInt(row_idx, pm_cols.cast_time_index);
    std::string cti_hint;
    if (cti) cti_hint = "(index " + std::to_string(cti) + ")";
    DrawResolvedField("Cast time", ResolveCastTime(cti),
                      cti_hint.empty() ? nullptr : cti_hint.c_str());

    DrawTextField("Cooldown (ms)", pm_cols.recovery_time,
                  spell_id, row_idx, DbcFieldType::UInt32);

    int64_t ri = GetCellInt(row_idx, pm_cols.range_index);
    std::string ri_hint;
    if (ri) ri_hint = "(index " + std::to_string(ri) + ")";
    DrawResolvedField("Range", ResolveRange(ri),
                      ri_hint.empty() ? nullptr : ri_hint.c_str());

    int64_t di = GetCellInt(row_idx, pm_cols.duration_index);
    std::string di_hint;
    if (di) di_hint = "(index " + std::to_string(di) + ")";
    DrawResolvedField("Duration", ResolveDuration(di),
                      di_hint.empty() ? nullptr : di_hint.c_str());

    ImGui::EndTable();
}

// Generic effect-section renderer. slot ∈ [0, 2] picks the column indices.
// Effect 1 is always rendered open; 2 and 3 collapse when their type=0 (empty).
void SpellEditorSystem::DrawEffectSection(int slot, int64_t spell_id, size_t row_idx) {
    // Effect column packs per slot.
    int eff_cols[3]  = { pm_cols.effect_1, pm_cols.effect_2, pm_cols.effect_3 };
    int base_cols[3] = { pm_cols.effect_base_points_1,
                         pm_cols.effect_base_points_2,
                         pm_cols.effect_base_points_3 };
    int die_cols[3]  = { pm_cols.effect_die_sides_1,
                         pm_cols.effect_die_sides_2,
                         pm_cols.effect_die_sides_3 };
    int amp_cols[3]  = { pm_cols.effect_amplitude_1,
                         pm_cols.effect_amplitude_2,
                         pm_cols.effect_amplitude_3 };
    int rad_cols[3]  = { pm_cols.effect_radius_index_1,
                         pm_cols.effect_radius_index_2,
                         pm_cols.effect_radius_index_3 };

    int eff_col   = eff_cols[slot];
    int base_col  = base_cols[slot];
    int die_col   = die_cols[slot];
    int amp_col   = amp_cols[slot];
    int rad_col   = rad_cols[slot];
    if (eff_col < 0) return;

    int64_t eff_type = GetCellInt(row_idx, eff_col);

    // Compose a section label that includes the effect's resolved name.
    const DbcEnum* eff_enum = GetDbcEnum("SpellEffects");
    std::string eff_label;
    if (eff_enum) {
        for (uint32_t i = 0; i < eff_enum->count; i++) {
            if (eff_enum->values[i].value == static_cast<int32_t>(eff_type)) {
                eff_label = eff_enum->values[i].label;
                break;
            }
        }
    }
    char header[96];
    if (slot == 0) {
        // Slot 1 is always present (most spells have at least one effect)
        if (eff_label.empty()) std::snprintf(header, sizeof(header), "Effect 1");
        else std::snprintf(header, sizeof(header), "Effect 1 - %s", eff_label.c_str());
    } else {
        if (eff_type == 0) {
            std::snprintf(header, sizeof(header), "Effect %d  (empty)", slot + 1);
        } else if (eff_label.empty()) {
            std::snprintf(header, sizeof(header), "Effect %d  - #%lld",
                          slot + 1, static_cast<long long>(eff_type));
        } else {
            std::snprintf(header, sizeof(header), "Effect %d  - %s",
                          slot + 1, eff_label.c_str());
        }
    }

    // Always show all 3 effect sections so the user can convert an empty
    // slot into an active effect by picking a Type. Slot 1 is expanded by
    // default; 2 and 3 collapse unless they have content.
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (slot == 0 || eff_type != 0) flags |= ImGuiTreeNodeFlags_DefaultOpen;

    ImGui::PushID(slot);
    if (!ImGui::CollapsingHeader(header, flags)) { ImGui::PopID(); return; }

    char table_id[16];
    std::snprintf(table_id, sizeof(table_id), "##eff%d", slot);
    if (!ImGui::BeginTable(table_id, 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) {
        ImGui::PopID();
        return;
    }
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 200);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    // Type combo. Picking a non-zero type from an empty slot effectively
    // "adds" an effect; picking 0 ("None") clears it back.
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Type");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);

        char preview[96];
        if (!eff_label.empty()) {
            std::snprintf(preview, sizeof(preview), "%s  (#%lld)",
                          eff_label.c_str(), static_cast<long long>(eff_type));
        } else {
            std::snprintf(preview, sizeof(preview), "#%lld",
                          static_cast<long long>(eff_type));
        }

        ImGui::PushID("type_combo");
        if (ImGui::BeginCombo("##type", preview)) {
            if (eff_enum) {
                for (uint32_t i = 0; i < eff_enum->count; i++) {
                    int32_t v = eff_enum->values[i].value;
                    bool selected = (v == static_cast<int32_t>(eff_type));
                    char item[96];
                    std::snprintf(item, sizeof(item), "%-32s  #%d",
                                  eff_enum->values[i].label, v);
                    if (ImGui::Selectable(item, selected)) {
                        Commit(spell_id, row_idx, pm_spells.columns[eff_col],
                               std::to_string(v), DbcFieldType::UInt32);
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();
    }

    DrawTextField("Base points", base_col, spell_id, row_idx, DbcFieldType::Int32);
    DrawTextField("Die sides",   die_col,  spell_id, row_idx, DbcFieldType::Int32);

    int64_t base = GetCellInt(row_idx, base_col);
    int64_t die  = GetCellInt(row_idx, die_col);
    if (die > 0) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%lld to %lld",
                      static_cast<long long>(base + 1),
                      static_cast<long long>(base + die));
        DrawResolvedField("Damage range", buf);
    }

    int64_t amp = GetCellInt(row_idx, amp_col);
    if (amp > 0) {
        DrawTextField("Tick period (ms)", amp_col, spell_id, row_idx, DbcFieldType::UInt32);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f sec", amp / 1000.0f);
        DrawResolvedField("  Resolved", buf);
    }

    int64_t radius_idx = GetCellInt(row_idx, rad_col);
    if (radius_idx > 0) {
        DrawResolvedField("Radius", ResolveRadius(radius_idx));
    }

    // Triggered spell (Branch Z) - if this effect triggers another spell,
    // render a clickable link so the user can navigate the chain.
    int64_t trig_id = GetCellInt(row_idx, pm_cols.effect_trigger_spell[slot]);
    if (trig_id > 0) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Triggers spell");
        ImGui::TableSetColumnIndex(1);
        SpellLink(trig_id);
    }

    ImGui::EndTable();
    ImGui::PopID();
}

// ---- Attributes section -----------------------------------------------------
//
// 8 bitmask columns (Attributes + AttributesEx + AttributesEx2..7). For each
// non-zero column we list the human-readable set-bit names using the
// matching SpellAttrN enum. Read-only display in v1 - editing happens via
// the regular DBC Browser cell.
void SpellEditorSystem::DrawAttributesSection(int64_t spell_id, size_t row_idx) {
    const char* col_labels[8] = {
        "Attributes",   "AttributesEx",  "AttributesEx2", "AttributesEx3",
        "AttributesEx4", "AttributesEx5", "AttributesEx6", "AttributesEx7"
    };
    const char* enum_names[8] = {
        "SpellAttr0", "SpellAttr1", "SpellAttr2", "SpellAttr3",
        "SpellAttr4", "SpellAttr5", "SpellAttr6", "SpellAttr7"
    };

    // Always show the section now so users can ADD attributes to a column
    // that's currently zero. Previously we hid empty columns which made it
    // impossible to set the first bit on a "clean" spell.
    if (!ImGui::CollapsingHeader("Attributes")) return;
    if (!ImGui::BeginTable("##attrs", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 140);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    for (int i = 0; i < 8; i++) {
        int col = pm_cols.attributes_attr[i];
        if (col < 0) continue;
        uint32_t mask = static_cast<uint32_t>(GetCellInt(row_idx, col));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", col_labels[i]);
        ImGui::TableSetColumnIndex(1);

        const DbcEnum* e = GetDbcEnum(enum_names[i]);

        // Summary of set bits + hex.
        std::ostringstream out;
        if (mask == 0) {
            out << "(none)";
        } else if (e) {
            bool first = true;
            for (uint32_t b = 0; b < e->count; b++) {
                uint32_t bit = static_cast<uint32_t>(e->values[b].value);
                if (mask & bit) {
                    if (!first) out << ", ";
                    out << e->values[b].label;
                    first = false;
                }
            }
            if (first) out << "(unknown bits)";
        } else {
            out << "(no enum)";
        }
        char hex[16];
        std::snprintf(hex, sizeof(hex), "0x%08X", mask);

        ImGui::PushID(i);
        ImGui::TextWrapped("%s", out.str().c_str());
        ImGui::TextDisabled("    %s", hex);
        ImGui::SameLine();
        if (ImGui::SmallButton("Edit...")) ImGui::OpenPopup("##attr_edit");

        // Popup with all 32 bit checkboxes for this attribute column. We
        // accumulate edits into `pending` and commit once at the end of
        // the frame if the popup-edited value differs from the row's value.
        if (ImGui::BeginPopup("##attr_edit")) {
            ImGui::Text("%s  (0x%08X)", col_labels[i], mask);
            ImGui::Separator();

            uint32_t edited = mask;

            // If the enum is registered, show its labeled flags grouped first.
            // Bits not covered by the enum get a generic "bit N (0xN)" entry
            // at the bottom so unknown flags remain editable.
            std::vector<bool> covered(32, false);
            if (e) {
                for (uint32_t b = 0; b < e->count; b++) {
                    uint32_t bit = static_cast<uint32_t>(e->values[b].value);
                    // Mark the bit position so we don't double-render it.
                    for (int p = 0; p < 32; p++) {
                        if (bit == (1u << p)) { covered[p] = true; break; }
                    }
                    bool on = (edited & bit) != 0;
                    char lbl[160];
                    std::snprintf(lbl, sizeof(lbl), "%s (0x%X)",
                                  e->values[b].label, bit);
                    if (ImGui::Checkbox(lbl, &on)) {
                        if (on) edited |= bit;
                        else    edited &= ~bit;
                    }
                }
            }

            // Uncovered bits at the bottom.
            bool printed_sep = false;
            for (int p = 0; p < 32; p++) {
                if (covered[p]) continue;
                uint32_t bit = 1u << p;
                bool on = (edited & bit) != 0;
                // Only render uncovered bits if they're set or if the
                // user explicitly toggles them. Hide the long tail of
                // unset, unnamed bits to keep the popup small.
                if (!on) continue;
                if (!printed_sep) {
                    ImGui::Separator();
                    ImGui::TextDisabled("Unmapped bits:");
                    printed_sep = true;
                }
                char lbl[32];
                std::snprintf(lbl, sizeof(lbl), "bit %d (0x%X)", p, bit);
                if (ImGui::Checkbox(lbl, &on)) {
                    if (on) edited |= bit;
                    else    edited &= ~bit;
                }
            }

            // Raw hex input as an escape hatch for setting unmapped bits.
            ImGui::Separator();
            char hex_buf[16];
            std::snprintf(hex_buf, sizeof(hex_buf), "%X", edited);
            ImGui::SetNextItemWidth(120);
            if (ImGui::InputText("Hex", hex_buf, sizeof(hex_buf),
                                 ImGuiInputTextFlags_CharsHexadecimal)) {
                edited = static_cast<uint32_t>(std::strtoul(hex_buf, nullptr, 16));
            }

            if (edited != mask) {
                Commit(spell_id, row_idx, pm_spells.columns[col],
                       std::to_string(edited), DbcFieldType::UInt32);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::EndTable();
}

// ---- Reagents section -------------------------------------------------------
//
// Up to 8 (item_id, count) pairs. v1 shows item ID + count only; item names
// require integrating with the server-side item_template table which is
// outside the DBC database. Will resolve in a follow-up.
void SpellEditorSystem::DrawReagentsSection(size_t row_idx) {
    bool any = false;
    for (int i = 0; i < 8; i++) {
        if (pm_cols.reagent[i] >= 0 &&
            GetCellInt(row_idx, pm_cols.reagent[i]) != 0) {
            any = true; break;
        }
    }
    if (!any) return;

    if (!ImGui::CollapsingHeader("Reagents")) return;
    if (!ImGui::BeginTable("##reagents", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 160);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    for (int i = 0; i < 8; i++) {
        int icol = pm_cols.reagent[i];
        int ccol = pm_cols.reagent_count[i];
        if (icol < 0) continue;
        int64_t item_id = GetCellInt(row_idx, icol);
        if (item_id == 0) continue;
        int64_t count = (ccol >= 0) ? GetCellInt(row_idx, ccol) : 0;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        char slot_label[24];
        std::snprintf(slot_label, sizeof(slot_label), "Reagent %d", i + 1);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", slot_label);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Item #%lld x %lld",
                    static_cast<long long>(item_id),
                    static_cast<long long>(count));
    }

    ImGui::TextDisabled("Item name lookup requires the item_template table - not "
                        "in the DBC database. Coming in a follow-up.");

    ImGui::EndTable();
}

// ---- Cooldown details section ----------------------------------------------
//
// Surfaces the spell's Category (cooldown grouping) and lists other spells
// in the same category. Categories are pre-indexed at load time.
void SpellEditorSystem::DrawCooldownDetailsSection(size_t row_idx) {
    if (pm_cols.category < 0) return;
    int64_t cat = GetCellInt(row_idx, pm_cols.category);
    if (cat == 0) return;  // no category = no shared cooldown

    if (!ImGui::CollapsingHeader("Cooldown details")) return;
    if (!ImGui::BeginTable("##cdetails", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 200);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    DrawResolvedField("Category", "#" + std::to_string(cat));

    auto it = pm_category_to_rows.find(cat);
    if (it != pm_category_to_rows.end()) {
        const auto& rows = it->second;
        int64_t self_id = GetCellInt(row_idx, pm_cols.id);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("Shares cooldown with");
        ImGui::TableSetColumnIndex(1);

        std::ostringstream names;
        int shown = 0;
        for (int r : rows) {
            int64_t other_id = GetCellInt(static_cast<size_t>(r), pm_cols.id);
            if (other_id == self_id) continue;
            std::string name = GetCell(static_cast<size_t>(r), pm_cols.name);
            if (name.empty()) continue;
            if (shown > 0) names << ", ";
            names << name;
            if (++shown >= 20) {
                names << ", +" << (static_cast<int>(rows.size()) - shown - 1) << " more";
                break;
            }
        }
        if (shown == 0) {
            ImGui::TextDisabled("(no other spells share this category)");
        } else {
            ImGui::TextWrapped("%s", names.str().c_str());
        }
    }

    ImGui::EndTable();
}

// ---- Modifier graph sections (Branch Z) -----------------------------------

void SpellEditorSystem::DrawConditionsLinksSection(int64_t /*spell_id*/,
                                                   size_t row_idx) {
    int64_t caster_aura      = GetCellInt(row_idx, pm_cols.caster_aura_spell);
    int64_t target_aura      = GetCellInt(row_idx, pm_cols.target_aura_spell);
    int64_t excl_caster_aura = GetCellInt(row_idx, pm_cols.exclude_caster_aura_spell);
    int64_t excl_target_aura = GetCellInt(row_idx, pm_cols.exclude_target_aura_spell);

    if (caster_aura == 0 && target_aura == 0 &&
        excl_caster_aura == 0 && excl_target_aura == 0) return;

    if (!ImGui::CollapsingHeader("Required / excluded auras")) return;
    if (!ImGui::BeginTable("##cond_links", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 200);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    auto link_row = [&](const char* label, int64_t id) {
        if (id == 0) return;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", label);
        ImGui::TableSetColumnIndex(1);
        SpellLink(id);
    };
    link_row("Requires caster aura",  caster_aura);
    link_row("Requires target aura",  target_aura);
    link_row("Excludes caster aura",  excl_caster_aura);
    link_row("Excludes target aura",  excl_target_aura);

    ImGui::EndTable();
}

void SpellEditorSystem::DrawReferencedBySection(int64_t spell_id) {
    auto it = pm_reverse_refs.find(spell_id);
    if (it == pm_reverse_refs.end() || it->second.empty()) return;

    if (!ImGui::CollapsingHeader("Referenced by")) return;

    // Group by kind so the user can see the relationship types at a glance.
    const char* kind_labels[] = {
        "Required caster aura for",   // CasterAura
        "Required target aura for",   // TargetAura
        "Excludes caster from",       // ExcludeCasterAura
        "Excludes target from",       // ExcludeTargetAura
        "Triggered (effect 1) by",    // TriggerSpell1
        "Triggered (effect 2) by",    // TriggerSpell2
        "Triggered (effect 3) by",    // TriggerSpell3
    };

    std::vector<std::vector<int>> by_kind(7);
    for (const auto& r : it->second) {
        int k = static_cast<int>(r.kind);
        if (k >= 0 && k < 7) by_kind[k].push_back(r.source_row);
    }

    constexpr int kCapPerGroup = 50;
    for (int k = 0; k < 7; k++) {
        if (by_kind[k].empty()) continue;
        ImGui::TextDisabled("%s  (%zu):", kind_labels[k], by_kind[k].size());
        ImGui::Indent();
        int shown = 0;
        for (int src_row : by_kind[k]) {
            if (shown >= kCapPerGroup) {
                ImGui::TextDisabled("...+%d more",
                                    static_cast<int>(by_kind[k].size()) - shown);
                break;
            }
            int64_t sid = GetCellInt(static_cast<size_t>(src_row), pm_cols.id);
            SpellLink(sid);
            shown++;
        }
        ImGui::Unindent();
    }
}

void SpellEditorSystem::DrawTalentsSection(int64_t spell_id) {
    auto it = pm_talent_refs.find(spell_id);
    if (it == pm_talent_refs.end() || it->second.empty()) return;

    if (!ImGui::CollapsingHeader("Talents")) return;

    int tab_col  = FindCol(pm_talents.columns, DbcColumnName("TabID").c_str());
    int id_col   = FindCol(pm_talents.columns, DbcColumnName("Id").c_str());
    int tier_col = FindCol(pm_talents.columns, DbcColumnName("TierID").c_str());

    constexpr int kCap = 50;
    int shown = 0;
    for (const auto& ref : it->second) {
        if (shown >= kCap) {
            ImGui::TextDisabled("...+%d more talents",
                                static_cast<int>(it->second.size()) - shown);
            break;
        }
        if (ref.talent_row < 0 ||
            ref.talent_row >= static_cast<int>(pm_talents.rows.size())) continue;
        const auto& tr = pm_talents.rows[ref.talent_row].values;

        int64_t talent_id = 0, tab_id = 0, tier = 0;
        if (id_col   >= 0 && id_col   < static_cast<int>(tr.size())) talent_id = std::strtoll(tr[id_col].c_str(),   nullptr, 10);
        if (tab_col  >= 0 && tab_col  < static_cast<int>(tr.size())) tab_id    = std::strtoll(tr[tab_col].c_str(),  nullptr, 10);
        if (tier_col >= 0 && tier_col < static_cast<int>(tr.size())) tier      = std::strtoll(tr[tier_col].c_str(), nullptr, 10);

        auto tn = pm_talent_tab_names.find(tab_id);
        const char* tab_name = (tn != pm_talent_tab_names.end()) ? tn->second.c_str() : "?";

        ImGui::BulletText("%s  -  Rank %d / tier %d  (talent #%lld)",
                          tab_name, ref.rank_index + 1,
                          static_cast<int>(tier),
                          static_cast<long long>(talent_id));
        shown++;
    }
}

void SpellEditorSystem::DrawGlyphsSection(int64_t spell_id) {
    auto it = pm_glyph_refs.find(spell_id);
    if (it == pm_glyph_refs.end() || it->second.empty()) return;

    if (!ImGui::CollapsingHeader("Glyphs")) return;

    for (int64_t glyph_id : it->second) {
        ImGui::BulletText("Glyph #%lld", static_cast<long long>(glyph_id));
    }
    ImGui::TextDisabled("Glyph items live in Item.dbc with GlyphPropertiesID;"
                        " cross-DBC navigation will come in a follow-up.");
}

void SpellEditorSystem::DrawFamilyModifiersSection(int64_t spell_id, size_t row_idx) {
    int64_t family = GetCellInt(row_idx, pm_cols.spell_family_name);
    if (family == 0) return;
    uint32_t flags = 0;
    for (int i = 0; i < 3; i++)
        flags |= static_cast<uint32_t>(GetCellInt(row_idx, pm_cols.spell_family_flags[i]));
    if (flags == 0) return;

    const auto& mods = GetFamilyModifiersFor(spell_id, row_idx);
    if (mods.empty()) return;

    char header[64];
    std::snprintf(header, sizeof(header), "Modified by  (%zu)", mods.size());
    if (!ImGui::CollapsingHeader(header)) return;

    ImGui::TextDisabled("Spells in family %lld whose effect class-mask "
                        "overlaps this spell's family flags.",
                        static_cast<long long>(family));
    ImGui::Separator();

    const DbcEnum* eff_enum = GetDbcEnum("SpellEffects");
    constexpr int kCap = 50;
    int shown = 0;
    for (const auto& m : mods) {
        if (shown >= kCap) {
            ImGui::TextDisabled("...+%d more modifiers",
                                static_cast<int>(mods.size()) - shown);
            break;
        }
        int src_row = m.first;
        int slot    = m.second;
        if (src_row < 0 ||
            src_row >= static_cast<int>(pm_spells.rows.size())) continue;
        int64_t sid = GetCellInt(static_cast<size_t>(src_row), pm_cols.id);

        int64_t eff_type = GetCellInt(static_cast<size_t>(src_row),
                                      slot == 0 ? pm_cols.effect_1 :
                                      slot == 1 ? pm_cols.effect_2 :
                                                  pm_cols.effect_3);
        std::string eff_name;
        if (eff_enum) {
            for (uint32_t i = 0; i < eff_enum->count; i++) {
                if (eff_enum->values[i].value == static_cast<int32_t>(eff_type)) {
                    eff_name = eff_enum->values[i].label;
                    break;
                }
            }
        }

        ImGui::PushID(src_row * 10 + slot);
        SpellLink(sid);
        ImGui::SameLine();
        if (!eff_name.empty()) {
            ImGui::TextDisabled("(effect %d - %s)", slot + 1, eff_name.c_str());
        } else {
            ImGui::TextDisabled("(effect %d - type %lld)",
                                slot + 1, static_cast<long long>(eff_type));
        }
        ImGui::PopID();
        shown++;
    }
}

void SpellEditorSystem::Commit(int64_t spell_id, size_t row_index,
                               const std::string& column,
                               const std::string& value,
                               DbcFieldType type) {
    if (!pm_db.UpdateCell("spell", "id", spell_id, column, value, type)) {
        pm_last_error = pm_db.LastError();
        return;
    }
    pm_last_error.clear();

    // Refresh just this row.
    DbConnection::Row fresh;
    if (pm_db.FetchRow("spell", "id", spell_id, fresh) &&
        fresh.values.size() == pm_spells.rows[row_index].values.size()) {
        pm_spells.rows[row_index] = std::move(fresh);
        if (pm_cols.name >= 0 &&
            pm_cols.name < static_cast<int>(pm_spells.rows[row_index].values.size())) {
            pm_lowercase_names[row_index] =
                ToLowerCopy(pm_spells.rows[row_index].values[pm_cols.name]);
        }
    }
}

} // namespace mve
