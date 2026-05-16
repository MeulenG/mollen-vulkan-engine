#include "spell_editor_system.h"
#include "dbc_naming.h"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

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
    c.id              = FindCol(cols, "id");
    c.name            = FindCol(cols, "spell_name_en_us");
    c.rank            = FindCol(cols, "rank_en_us");
    c.description     = FindCol(cols, "description_en_us");
    c.icon_id         = FindCol(cols, "spell_icon_id");
    c.mana_cost       = FindCol(cols, "mana_cost");
    c.cast_time_index = FindCol(cols, "casting_time_index");
    c.range_index     = FindCol(cols, "range_index");
    c.recovery_time   = FindCol(cols, "recovery_time");
    c.school_mask     = FindCol(cols, "school_mask");
    c.spell_family    = FindCol(cols, "spell_family_name");
    c.effect_1        = FindCol(cols, "effect1");
    c.effect_base_points_1 = FindCol(cols, "effect_base_points1");
    c.effect_die_sides_1   = FindCol(cols, "effect_die_sides1");
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

    size_t row_idx = static_cast<size_t>(pm_selected_row);
    int64_t spell_id = GetCellInt(row_idx, pm_cols.id);

    // Lazy-init buffers for this row's editable cells.
    auto ensure_buf = [&](int col) {
        if (col < 0) return;
        auto it = pm_edit_buffers.find(col);
        if (it == pm_edit_buffers.end()) {
            pm_edit_buffers[col] = GetCell(row_idx, col);
        }
    };
    ensure_buf(pm_cols.name);
    ensure_buf(pm_cols.rank);
    ensure_buf(pm_cols.description);
    ensure_buf(pm_cols.mana_cost);
    ensure_buf(pm_cols.recovery_time);
    ensure_buf(pm_cols.effect_base_points_1);
    ensure_buf(pm_cols.effect_die_sides_1);

    // ---- Header: big icon + name/rank + meta ----
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
    if (!rank.empty()) {
        ImGui::TextDisabled("%s", rank.c_str());
    }
    int64_t mask = GetCellInt(row_idx, pm_cols.school_mask);
    ImGui::TextDisabled("School: %s    #%lld",
                        SchoolNameFromMask(mask),
                        static_cast<long long>(spell_id));
    ImGui::EndGroup();

    ImGui::Separator();

    if (!pm_last_error.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", pm_last_error.c_str());
    }

    // Helper to render a "Label: editor" row inside the current table.
    auto draw_field_str = [&](const char* label, int col, DbcFieldType type) {
        if (col < 0) return;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);

        std::string& buf = pm_edit_buffers[col];
        if (buf.capacity() < 256) buf.reserve(256);
        ImGui::PushID(col);
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
            Commit(spell_id, row_idx, pm_spells.columns[col], val, type);
        }
    };

    // ---- Identity card ----
    if (ImGui::CollapsingHeader("Identity", ImGuiTreeNodeFlags_DefaultOpen) &&
        ImGui::BeginTable("##identity", 2,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH)) {
        ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);
        draw_field_str("Name",        pm_cols.name,        DbcFieldType::String);
        draw_field_str("Rank",        pm_cols.rank,        DbcFieldType::String);
        ImGui::EndTable();
    }

    // ---- Description card ----
    if (pm_cols.description >= 0 &&
        ImGui::CollapsingHeader("Description", ImGuiTreeNodeFlags_DefaultOpen)) {
        std::string& buf = pm_edit_buffers[pm_cols.description];
        if (buf.capacity() < 2048) buf.reserve(2048);
        ImGui::PushID(pm_cols.description);
        ImGui::InputTextMultiline(
            "##desc", buf.data(), buf.capacity(),
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4),
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
            std::string val = buf.c_str();
            Commit(spell_id, row_idx, pm_spells.columns[pm_cols.description],
                   val, DbcFieldType::String);
        }
        ImGui::TextDisabled("$s1/$s2 = effect points,  $o1 = over-time amount,  $d = duration");
        ImGui::PopID();
    }

    // ---- Cost & Cast card ----
    if (ImGui::CollapsingHeader("Cost & Cast", ImGuiTreeNodeFlags_DefaultOpen) &&
        ImGui::BeginTable("##castcost", 2,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH)) {
        ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);
        draw_field_str("Mana cost",     pm_cols.mana_cost,     DbcFieldType::UInt32);
        draw_field_str("Cooldown (ms)", pm_cols.recovery_time, DbcFieldType::UInt32);
        ImGui::EndTable();
    }

    // ---- Effect 1 card ----
    if (ImGui::CollapsingHeader("Effect 1", ImGuiTreeNodeFlags_DefaultOpen) &&
        ImGui::BeginTable("##effect1", 2,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH)) {
        ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 110);
        ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);

        // Show the raw effect type as int for now — the SpellEffect enum
        // table would slot in here once tagged.
        int64_t eff_type = GetCellInt(row_idx, pm_cols.effect_1);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Type");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%lld", static_cast<long long>(eff_type));

        draw_field_str("Base points",  pm_cols.effect_base_points_1, DbcFieldType::Int32);
        draw_field_str("Die sides",    pm_cols.effect_die_sides_1,   DbcFieldType::Int32);

        // Computed damage range
        int64_t base = GetCellInt(row_idx, pm_cols.effect_base_points_1);
        int64_t die  = GetCellInt(row_idx, pm_cols.effect_die_sides_1);
        if (die > 0) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("Damage range");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("%lld to %lld",
                                static_cast<long long>(base + 1),
                                static_cast<long long>(base + die));
        }
        ImGui::EndTable();
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
