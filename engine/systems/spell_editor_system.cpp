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

    // Effect slots 2 and 3 — read-only for now (we substitute $s2/$o3 etc.
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
// We don't try to be fully faithful to the WoW client — the goal is "show
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

        // Unknown token — pass through verbatim so the user can see it.
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
    // contiguous index range — with filtering active, we need to build a
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

    // Lazy-init buffers for editable cells. Per-row buffers are wiped when
    // the selection changes (cleared in the finder's click handler).
    auto ensure_buf = [&](int col) {
        if (col < 0) return;
        if (pm_edit_buffers.find(col) == pm_edit_buffers.end()) {
            pm_edit_buffers[col] = GetCell(row_idx, col);
        }
    };
    int editable_cols[] = {
        pm_cols.name, pm_cols.rank, pm_cols.description,
        pm_cols.mana_cost, pm_cols.mana_cost_percentage,
        pm_cols.mana_cost_per_level, pm_cols.mana_per_second,
        pm_cols.mana_per_second_per_level,
        pm_cols.recovery_time,
        pm_cols.effect_base_points_1, pm_cols.effect_die_sides_1,
        pm_cols.effect_amplitude_1,
    };
    for (int c : editable_cols) ensure_buf(c);

    DrawHeaderSection(row_idx, spell_id);
    ImGui::Separator();

    if (!pm_last_error.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", pm_last_error.c_str());
        ImGui::Separator();
    }

    DrawIdentitySection(spell_id, row_idx);
    DrawDescriptionSection(spell_id, row_idx);
    DrawCostCastSection(spell_id, row_idx);
    DrawEffect1Section(spell_id, row_idx);
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
                                      int64_t spell_id, size_t /*row_idx*/,
                                      DbcFieldType type) {
    if (col_index < 0) return false;
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);

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

void SpellEditorSystem::DrawEffect1Section(int64_t spell_id, size_t row_idx) {
    if (!ImGui::CollapsingHeader("Effect 1", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (!ImGui::BeginTable("##effect1", 2,
                           ImGuiTableFlags_SizingFixedFit |
                           ImGuiTableFlags_BordersInnerH)) return;
    ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 200);
    ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

    // Effect type — resolve via SpellEffects enum.
    int64_t eff_type = GetCellInt(row_idx, pm_cols.effect_1);
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
    char eff_str[96];
    if (!eff_label.empty()) {
        std::snprintf(eff_str, sizeof(eff_str), "%s  (#%lld)",
                      eff_label.c_str(), static_cast<long long>(eff_type));
    } else {
        std::snprintf(eff_str, sizeof(eff_str), "#%lld",
                      static_cast<long long>(eff_type));
    }
    DrawResolvedField("Type", eff_str);

    DrawTextField("Base points", pm_cols.effect_base_points_1,
                  spell_id, row_idx, DbcFieldType::Int32);
    DrawTextField("Die sides",   pm_cols.effect_die_sides_1,
                  spell_id, row_idx, DbcFieldType::Int32);

    int64_t base = GetCellInt(row_idx, pm_cols.effect_base_points_1);
    int64_t die  = GetCellInt(row_idx, pm_cols.effect_die_sides_1);
    if (die > 0) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%lld to %lld",
                      static_cast<long long>(base + 1),
                      static_cast<long long>(base + die));
        DrawResolvedField("Damage range", buf);
    }

    int64_t amp = GetCellInt(row_idx, pm_cols.effect_amplitude_1);
    if (amp > 0) {
        DrawTextField("Tick period (ms)", pm_cols.effect_amplitude_1,
                      spell_id, row_idx, DbcFieldType::UInt32);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f sec", amp / 1000.0f);
        DrawResolvedField("  Resolved", buf);
    }

    int64_t radius_idx = GetCellInt(row_idx, pm_cols.effect_radius_index_1);
    if (radius_idx > 0) {
        DrawResolvedField("Radius", ResolveRadius(radius_idx));
    }

    ImGui::EndTable();
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
