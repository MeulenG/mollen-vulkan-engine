#include "dbc_browser_system.h"
#include "dbc_naming.h"
#include "schema_registry.h"
#include "enum_registry.h"

#include <imgui.h>

#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdlib>

namespace mve {

namespace {

bool IsPaddingField(const char* name) {
    return name && name[0] == '_' && std::strncmp(name, "_pad", 4) == 0;
}

bool MatchesFilter(const std::string& haystack, const char* filter) {
    if (!filter || !*filter) return true;
    auto to_lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::string n; n.reserve(haystack.size());
    for (unsigned char c : haystack) n.push_back(to_lower(c));
    std::string f; f.reserve(std::strlen(filter));
    for (const char* p = filter; *p; ++p) f.push_back(to_lower(static_cast<unsigned char>(*p)));
    return n.find(f) != std::string::npos;
}

// Default column width per data type. Values are deliberately conservative so
// the table fits a lot of columns horizontally without scrolling for the
// common case (id-heavy tables). String columns are wider because most of
// them carry display names.
float ColumnWidthForType(DbcFieldType type) {
    switch (type) {
    case DbcFieldType::String:                                return 220.0f;
    case DbcFieldType::Float:                                 return  90.0f;
    case DbcFieldType::UInt32:
    case DbcFieldType::Int32:                                 return  90.0f;
    case DbcFieldType::UInt8:
    case DbcFieldType::Int8:
    case DbcFieldType::UInt16:
    case DbcFieldType::Int16:                                 return  70.0f;
    }
    return 90.0f;
}

// Semantic-aware column width. Used for the editable PSQL table where
// friendly widgets (combos, FK labels, "[3 set] 0x1A" buttons) need more
// horizontal room than the raw integer they replace. File mode keeps the
// narrower type-only widths since cells there are still raw values.
float ColumnWidthForField(const DbcFieldDef& f) {
    switch (f.semantic) {
    case DbcSemantic::ForeignKey: return 200.0f;  // "Some Long Name (123)"
    case DbcSemantic::Enum:       return 150.0f;  // "Held in Off-Hand"
    case DbcSemantic::Bitmask:    return 130.0f;  // "[5 set] 0x1A2B"
    case DbcSemantic::Color:      return 110.0f;  // swatch + small preview
    case DbcSemantic::Boolean:    return  60.0f;  // checkbox only
    case DbcSemantic::LocalizedString:
    case DbcSemantic::Default:
    default: break;
    }
    return ColumnWidthForType(f.type);
}

// If the last item drawn was clipped (rendered text wider than its cell),
// show its full content as a tooltip on hover. Cheap to call after every
// cell because IsItemHovered short-circuits in the common case.
void TooltipIfHovered(const char* full_text) {
    if (!full_text || !*full_text) return;
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal |
                             ImGuiHoveredFlags_NoSharedDelay)) {
        ImGui::SetTooltip("%s", full_text);
    }
}

// ---- File-mode cell rendering (read-only, mirrors original behavior) ----

// Render a cell's text + a tooltip with the same content. The tooltip is
// what makes long values discoverable when the column clips them.
void DrawTextCell(const char* text) {
    ImGui::TextUnformatted(text);
    TooltipIfHovered(text);
}

void DrawNonPackedFileCell(const DbcFile& dbc, const DbcFieldDef& field,
                           uint32_t record, uint32_t field_index) {
    char buf[256];
    switch (field.type) {
    case DbcFieldType::UInt32:
    case DbcFieldType::UInt8:
    case DbcFieldType::UInt16:
        std::snprintf(buf, sizeof(buf), "%u", dbc.GetUInt32(record, field_index));
        DrawTextCell(buf);
        break;
    case DbcFieldType::Int32:
    case DbcFieldType::Int8:
    case DbcFieldType::Int16:
        std::snprintf(buf, sizeof(buf), "%d", dbc.GetInt32(record, field_index));
        DrawTextCell(buf);
        break;
    case DbcFieldType::Float:
        std::snprintf(buf, sizeof(buf), "%.3f", dbc.GetFloat(record, field_index));
        DrawTextCell(buf);
        break;
    case DbcFieldType::String: {
        const char* s = dbc.GetStringField(record, field_index);
        DrawTextCell(s ? s : "");
        break;
    }
    }
}

void DrawPackedFileCell(const DbcFile& dbc, const DbcSchema* schema,
                        uint32_t record, uint32_t field_index) {
    uint32_t offset = DbcFile::GetFieldOffset(schema, field_index);
    const auto& field = schema->fields[field_index];
    char buf[256];
    switch (field.type) {
    case DbcFieldType::UInt8:
        std::snprintf(buf, sizeof(buf), "%u", dbc.GetUInt8At(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::Int8:
        std::snprintf(buf, sizeof(buf), "%d", dbc.GetInt8At(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::UInt16:
        std::snprintf(buf, sizeof(buf), "%u", dbc.GetUInt16At(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::Int16:
        std::snprintf(buf, sizeof(buf), "%d", dbc.GetInt16At(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::UInt32:
        std::snprintf(buf, sizeof(buf), "%u", dbc.GetUInt32At(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::Int32:
        std::snprintf(buf, sizeof(buf), "%d", dbc.GetInt32At(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::Float:
        std::snprintf(buf, sizeof(buf), "%.3f", dbc.GetFloatAt(record, offset));
        DrawTextCell(buf); break;
    case DbcFieldType::String: {
        const char* s = dbc.GetStringAt(record, offset);
        DrawTextCell(s ? s : "");
        break;
    }
    }
}

} // namespace

DbcBrowserSystem::DbcBrowserSystem(DbcRegistry& registry, DbConnection& db)
    : pm_registry{registry}, pm_db{db} {}

DbcBrowserSystem::SourceMode DbcBrowserSystem::SourceFor(const std::string& name) const {
    if (!pm_db.IsConnected()) return SourceMode::File;
    if (!pm_registry.HasSchema(name)) return SourceMode::File;
    if (!pm_db.TableExists(DbcTableName(name.c_str()))) return SourceMode::File;
    return SourceMode::Psql;
}

DbConnection::Table* DbcBrowserSystem::GetCachedTable(const std::string& dbc_name) {
    auto it = pm_psql_cache.find(dbc_name);
    if (it != pm_psql_cache.end()) return &it->second;

    DbConnection::Table tbl;
    if (!pm_db.FetchTable(DbcTableName(dbc_name.c_str()), tbl)) return nullptr;

    auto [ins, _] = pm_psql_cache.emplace(dbc_name, std::move(tbl));
    return &ins->second;
}

void DbcBrowserSystem::InvalidateTable(const std::string& dbc_name) {
    pm_psql_cache.erase(dbc_name);
}

void DbcBrowserSystem::Update() {
    ImGui::Begin("DBC Browser");

    DrawConnectionHeader();
    ImGui::Separator();

    const auto& names = pm_registry.AvailableNames();
    if (names.empty() && !pm_db.IsConnected()) {
        ImGui::TextWrapped("No .dbc files in: %s",
                           pm_registry.DbcDir().string().c_str());
        ImGui::TextWrapped("Run dbc-extract --export to populate, or connect "
                           "to PostgreSQL to read from the database.");
        ImGui::End();
        return;
    }

    float list_width = 280.0f;
    ImGui::BeginChild("DbcList", ImVec2(list_width, 0), ImGuiChildFlags_Borders);
    DrawDbcList();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("DbcTable", ImVec2(0, 0), ImGuiChildFlags_Borders);
    DrawRecordTable();
    ImGui::EndChild();

    ImGui::End();
}

void DbcBrowserSystem::DrawConnectionHeader() {
    if (pm_db.IsConnected()) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1), "[connected]");
        ImGui::SameLine();
        ImGui::Text("%s @ %s:%d",
                    pm_db.DbName().c_str(),
                    pm_db.Host().c_str(),
                    pm_db.Port());
        ImGui::SameLine();
        if (ImGui::SmallButton("Refresh schema")) {
            pm_db.RefreshSchema();
            pm_psql_cache.clear();
        }
    } else {
        ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.4f, 1), "[disconnected]");
        ImGui::SameLine();
        ImGui::TextDisabled("file mode (read-only) — %s",
                            pm_db.LastError().empty() ? "" : pm_db.LastError().c_str());
    }
}

void DbcBrowserSystem::DrawDbcList() {
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##filter", "Filter (matches pretty or raw name)...",
                             pm_filter, sizeof(pm_filter));
    ImGui::Separator();

    for (const auto& name : pm_registry.AvailableNames()) {
        std::string pretty = DbcPrettyName(name.c_str());
        // Filter applies to both forms — typing "creature" finds
        // "Creature Display Info" and the raw "CreatureDisplayInfo".
        if (!MatchesFilter(name, pm_filter) &&
            !MatchesFilter(pretty, pm_filter)) continue;

        SourceMode mode = SourceFor(name);
        bool selected = (name == pm_selected);
        bool has_schema = pm_registry.HasSchema(name);

        // Source-coded labels: green = psql (editable), white = file, grey = no schema.
        ImVec4 color;
        const char* badge;
        if (mode == SourceMode::Psql) {
            color = ImVec4(0.4f, 0.9f, 0.4f, 1.0f);
            badge = "[psql]";
        } else if (has_schema) {
            color = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
            badge = "[file]";
        } else {
            color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
            badge = "[raw] ";
        }

        // Use ##unique-id form so two pretty names that collide (rare but
        // possible after splitting) don't break selection.
        std::string label = badge + std::string(" ") + pretty + "##" + name;
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        if (ImGui::Selectable(label.c_str(), selected)) {
            pm_selected = name;
            pm_edit.column = -1;  // cancel any in-progress edit
        }
        ImGui::PopStyleColor();

        // Hover -> show raw schema name + table name. Helps when writing
        // SQL or reading server logs that use the canonical names.
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s\nSQL table: %s",
                              name.c_str(),
                              DbcTableName(name.c_str()).c_str());
        }
    }
}

void DbcBrowserSystem::DrawRecordTable() {
    if (pm_selected.empty()) {
        ImGui::TextDisabled("Select a DBC from the list to view its records.");
        return;
    }

    auto* entry = pm_registry.Load(pm_selected);
    if (!entry) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Entry not found in registry.");
        return;
    }
    if (entry->load_failed || !entry->file) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
                           "Failed to load %s", entry->path.string().c_str());
        return;
    }

    SourceMode mode = SourceFor(pm_selected);

    // Pretty name as the heading; raw schema + SQL table as muted subtitles
    // aligned to the right.
    std::string pretty = DbcPrettyName(pm_selected.c_str());
    std::string sql_table = DbcTableName(pm_selected.c_str());

    ImGui::PushFont(nullptr);  // (no big font yet — placeholder for future styling)
    ImGui::TextUnformatted(pretty.c_str());
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(8, 0));
    ImGui::SameLine();
    ImGui::TextDisabled("schema: %s   sql: %s",
                        pm_selected.c_str(), sql_table.c_str());

    ImGui::Text("Records: %u   Fields: %u   Record size: %u bytes",
                entry->file->GetRecordCount(),
                entry->file->GetFieldCount(),
                entry->file->GetRecordSize());

    if (mode == SourceMode::Psql) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1),
                           "Source: PostgreSQL  (double-click a cell to edit)");
    } else if (!entry->schema) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1),
                           "Source: file (raw uint32 columns, no schema)");
    } else {
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1),
                           "Source: file (read-only)");
    }

    ImGui::Separator();

    if (mode == SourceMode::Psql) {
        auto* tbl = GetCachedTable(pm_selected);
        if (!tbl) {
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
                               "Failed to fetch table: %s",
                               pm_db.LastError().c_str());
            if (ImGui::Button("Retry")) InvalidateTable(pm_selected);
            return;
        }
        if (ImGui::SmallButton("Refresh table")) {
            InvalidateTable(pm_selected);
            pm_edit.column = -1;
        }
        DrawPsqlTable(*entry, *tbl);
    } else {
        DrawFileTable(*entry);
    }
}

void DbcBrowserSystem::DrawPsqlTable(DbcRegistry::Entry& entry,
                                     DbConnection::Table& table) {
    const DbcSchema* schema = entry.schema;
    if (!schema) {
        ImGui::TextDisabled("No schema; cannot edit.");
        return;
    }

    // Build display column list from the schema (skipping padding) and map
    // to indices in `table.columns` by name. If a column is missing in the
    // DB (e.g. older import), it's shown as "(missing)".
    struct ColView {
        int field_index;        // index into schema->fields
        int db_column;          // index into table.columns; -1 if missing
        std::string col_name;   // snake_case
    };
    std::vector<ColView> view;
    view.reserve(schema->field_count);
    for (uint32_t f = 0; f < schema->field_count; f++) {
        if (IsPaddingField(schema->fields[f].name)) continue;
        ColView v;
        v.field_index = static_cast<int>(f);
        v.col_name = DbcColumnName(schema->fields[f].name);
        v.db_column = -1;
        for (size_t i = 0; i < table.columns.size(); i++) {
            if (table.columns[i] == v.col_name) {
                v.db_column = static_cast<int>(i);
                break;
            }
        }
        view.push_back(std::move(v));
    }

    if (view.empty()) {
        ImGui::TextDisabled("No editable columns.");
        return;
    }

    // Find the "id" column once for UPDATE/refresh paths.
    int id_col = -1;
    for (size_t i = 0; i < table.columns.size(); i++) {
        if (table.columns[i] == "id") { id_col = static_cast<int>(i); break; }
    }

    if (!pm_edit.last_error.empty() && pm_edit.dbc == pm_selected) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", pm_edit.last_error.c_str());
    }

    constexpr ImGuiTableFlags kTableFlags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

    if (!ImGui::BeginTable("##psql_records", static_cast<int>(view.size()), kTableFlags)) {
        return;
    }

    for (const auto& v : view) {
        const auto& field = schema->fields[v.field_index];
        ImGui::TableSetupColumn(field.name,
                                ImGuiTableColumnFlags_WidthFixed,
                                ColumnWidthForField(field));
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(table.rows.size()));
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
            ImGui::TableNextRow();
            const auto& db_row = table.rows[row];

            int64_t row_id = 0;
            if (id_col >= 0 && id_col < static_cast<int>(db_row.values.size())) {
                row_id = std::strtoll(db_row.values[id_col].c_str(), nullptr, 10);
            }

            for (size_t cv = 0; cv < view.size(); cv++) {
                ImGui::TableSetColumnIndex(static_cast<int>(cv));
                const auto& v = view[cv];

                if (v.db_column < 0 ||
                    v.db_column >= static_cast<int>(db_row.values.size())) {
                    ImGui::TextDisabled("(missing)");
                    continue;
                }

                ImGui::PushID(row);
                ImGui::PushID(static_cast<int>(cv));
                DrawPsqlCell(schema, v.field_index, table,
                             row, static_cast<int>(row_id),
                             v.db_column, static_cast<int>(cv));
                ImGui::PopID();
                ImGui::PopID();
            }
        }
    }
    ImGui::EndTable();
}

// ---- FK label cache ---------------------------------------------------------

const std::string& DbcBrowserSystem::ResolveFkLabel(
        const std::string& target_table, int64_t id) {

    static const std::string kEmpty;

    auto& cache = pm_fk_cache[target_table];
    if (!cache.resolved) {
        cache.resolved = true;  // mark first to avoid re-querying on failure

        // Pull the whole target table once. For huge tables (Spell ~50k
        // rows) this is a one-time cost paid lazily.
        DbConnection::Table tbl;
        if (pm_db.FetchTable(target_table, tbl)) {
            // Pick the column that gives a human-readable label. Most DBC
            // tables expose a localized name; some non-localized tables
            // use a plain "name" column. Fall back to the second column
            // by position if neither exists.
            int label_col = -1;
            const char* preferred[] = { "name_enus", "name_lang", "name" };
            for (const char* p : preferred) {
                for (size_t c = 0; c < tbl.columns.size(); c++) {
                    if (tbl.columns[c] == p) {
                        label_col = static_cast<int>(c);
                        break;
                    }
                }
                if (label_col >= 0) break;
            }
            int id_col_idx = -1;
            for (size_t c = 0; c < tbl.columns.size(); c++) {
                if (tbl.columns[c] == "id") {
                    id_col_idx = static_cast<int>(c);
                    break;
                }
            }
            if (id_col_idx >= 0) {
                for (const auto& r : tbl.rows) {
                    if (id_col_idx >= static_cast<int>(r.values.size())) continue;
                    int64_t key = std::strtoll(r.values[id_col_idx].c_str(),
                                               nullptr, 10);
                    std::string label;
                    if (label_col >= 0 &&
                        label_col < static_cast<int>(r.values.size())) {
                        label = r.values[label_col];
                    }
                    cache.id_to_label[key] = std::move(label);
                }
            }
        }
    }

    auto it = cache.id_to_label.find(id);
    if (it == cache.id_to_label.end()) return kEmpty;
    return it->second;
}

// ---- per-cell semantic dispatch --------------------------------------------

void DbcBrowserSystem::DrawPsqlCell(const DbcSchema* schema,
                                    int field_index,
                                    DbConnection::Table& table,
                                    int row, int row_id, int db_column,
                                    int cv) {
    const auto& field = schema->fields[field_index];
    const std::string& val = table.rows[row].values[db_column];
    const std::string sql_table = DbcTableName(pm_selected.c_str());
    const std::string col_name = DbcColumnName(field.name);

    auto commit = [&](const std::string& new_value) {
        bool ok = pm_db.UpdateCell(sql_table, "id", row_id,
                                    col_name, new_value, field.type);
        if (ok) {
            DbConnection::Row fresh;
            if (pm_db.FetchRow(sql_table, "id", row_id, fresh) &&
                fresh.values.size() == table.rows[row].values.size()) {
                table.rows[row] = std::move(fresh);
            } else {
                InvalidateTable(pm_selected);
            }
            pm_edit.last_error.clear();
        } else {
            pm_edit.last_error = pm_db.LastError();
        }
    };

    // The primary key column stays read-only — editing it would orphan
    // every row that points at it, and we don't have cascading update
    // support yet.
    if (std::strcmp(field.name, "Id") == 0) {
        ImGui::TextUnformatted(val.c_str());
        TooltipIfHovered(val.c_str());
        return;
    }

    switch (field.semantic) {

    // ---- Boolean: checkbox -------------------------------------------------
    case DbcSemantic::Boolean: {
        bool b = (val == "1" || val == "true" || val == "t");
        if (ImGui::Checkbox("##b", &b)) {
            commit(b ? "1" : "0");
        }
        break;
    }

    // ---- Enum: combo from registry ----------------------------------------
    case DbcSemantic::Enum: {
        const DbcEnum* e = field.hint ? GetDbcEnum(field.hint) : nullptr;
        if (!e) {
            // Unknown enum target — fall through to default text editing.
            goto default_cell;
        }
        int current = static_cast<int>(std::strtol(val.c_str(), nullptr, 10));
        const char* preview = "(unknown)";
        for (uint32_t i = 0; i < e->count; i++) {
            if (e->values[i].value == current) { preview = e->values[i].label; break; }
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##enum", preview)) {
            for (uint32_t i = 0; i < e->count; i++) {
                bool selected = (e->values[i].value == current);
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%d", e->values[i].value);
                std::string item_label = e->values[i].label;
                item_label += "  (";
                item_label += buf;
                item_label += ")";
                if (ImGui::Selectable(item_label.c_str(), selected)) {
                    if (e->values[i].value != current) {
                        commit(buf);
                    }
                }
            }
            ImGui::EndCombo();
        }
        break;
    }

    // ---- Color: RGBA8 packed in uint32 ------------------------------------
    case DbcSemantic::Color: {
        uint32_t packed = static_cast<uint32_t>(
            std::strtoul(val.c_str(), nullptr, 10));
        // WoW packs colors as BGRA in the low-to-high byte order — i.e.
        // the int's least-significant byte is Blue. Keep it consistent
        // with how the client renders.
        float rgba[4] = {
            ((packed >>  0) & 0xFF) / 255.0f,
            ((packed >>  8) & 0xFF) / 255.0f,
            ((packed >> 16) & 0xFF) / 255.0f,
            ((packed >> 24) & 0xFF) / 255.0f,
        };
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::ColorEdit4("##color", rgba,
                              ImGuiColorEditFlags_NoInputs |
                              ImGuiColorEditFlags_AlphaBar)) {
            uint32_t b = static_cast<uint32_t>(rgba[0] * 255.0f) & 0xFF;
            uint32_t g = static_cast<uint32_t>(rgba[1] * 255.0f) & 0xFF;
            uint32_t r = static_cast<uint32_t>(rgba[2] * 255.0f) & 0xFF;
            uint32_t a = static_cast<uint32_t>(rgba[3] * 255.0f) & 0xFF;
            uint32_t out = b | (g << 8) | (r << 16) | (a << 24);
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%u", out);
            commit(buf);
        }
        break;
    }

    // ---- Bitmask: button shows count + hex; popup has flag checkboxes -----
    case DbcSemantic::Bitmask: {
        uint32_t mask = static_cast<uint32_t>(
            std::strtoul(val.c_str(), nullptr, 10));
        const DbcEnum* e = field.hint ? GetDbcEnum(field.hint) : nullptr;

        // Count set bits + format a compact label like "[3 set] 0x1A".
        int set_count = 0;
        uint32_t m = mask;
        while (m) { set_count += (m & 1); m >>= 1; }
        char label[64];
        if (mask == 0) {
            std::snprintf(label, sizeof(label), "(none)");
        } else {
            std::snprintf(label, sizeof(label), "[%d set] 0x%X", set_count, mask);
        }
        if (ImGui::Button(label, ImVec2(-FLT_MIN, 0))) {
            ImGui::OpenPopup("##bitmask_popup");
        }
        if (ImGui::BeginPopup("##bitmask_popup")) {
            uint32_t edited = mask;
            if (e) {
                // Named flag table — show one labelled checkbox per known flag.
                for (uint32_t i = 0; i < e->count; i++) {
                    uint32_t bit = static_cast<uint32_t>(e->values[i].value);
                    bool on = (edited & bit) != 0;
                    if (ImGui::Checkbox(e->values[i].label, &on)) {
                        if (on) edited |= bit;
                        else    edited &= ~bit;
                    }
                }
            } else {
                // No registered flag table. Default view is compact:
                //   1. Hex input — direct edit for power users
                //   2. Set-bits list — click to clear individual set bits
                //   3. Collapsing "All 32 bits" — set previously-unset bits
                //      without scrolling a 32-row checkbox grid every time.
                char hex_buf[16];
                std::snprintf(hex_buf, sizeof(hex_buf), "0x%X", edited);
                ImGui::SetNextItemWidth(120);
                if (ImGui::InputText("Hex", hex_buf, sizeof(hex_buf),
                        ImGuiInputTextFlags_CharsHexadecimal |
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
                    edited = static_cast<uint32_t>(
                        std::strtoul(hex_buf, nullptr, 16));
                }

                ImGui::Separator();
                ImGui::TextDisabled("Set bits");
                bool any_set = false;
                for (int b = 0; b < 32; b++) {
                    uint32_t bit = 1u << b;
                    if (!(edited & bit)) continue;
                    any_set = true;
                    bool on = true;
                    char bn[32];
                    std::snprintf(bn, sizeof(bn), "bit %d (0x%X)", b, bit);
                    if (ImGui::Checkbox(bn, &on) && !on) {
                        edited &= ~bit;
                    }
                }
                if (!any_set) ImGui::TextDisabled("  (none)");

                ImGui::Separator();
                if (ImGui::CollapsingHeader("All 32 bits")) {
                    for (int b = 0; b < 32; b++) {
                        uint32_t bit = 1u << b;
                        bool on = (edited & bit) != 0;
                        char bn[32];
                        std::snprintf(bn, sizeof(bn), "bit %d (0x%X)", b, bit);
                        if (ImGui::Checkbox(bn, &on)) {
                            if (on) edited |= bit;
                            else    edited &= ~bit;
                        }
                    }
                }
            }
            if (edited != mask) {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%u", edited);
                commit(buf);
            }
            ImGui::EndPopup();
        }
        break;
    }

    // ---- Foreign key: "Label (id)" text, edit via plain InputText fallback
    case DbcSemantic::ForeignKey: {
        // The auto-tagger guesses target table names from field-name prefixes
        // and is wrong for many fields (e.g. "VariationID" -> "variation"
        // which doesn't exist). If the target isn't a real table in the DB,
        // we fall through to plain integer rendering with NO misleading
        // tooltip about a fictional FK.
        if (!field.hint || !pm_db.TableExists(field.hint)) {
            goto default_cell;
        }
        int64_t id_value = std::strtoll(val.c_str(), nullptr, 10);
        std::string display;
        if (id_value != 0) {
            const std::string& label = ResolveFkLabel(field.hint, id_value);
            if (!label.empty()) {
                display = label + "  (" + val + ")";
            }
        }
        if (display.empty()) display = val.empty() ? "0" : val;

        bool editing = (pm_edit.dbc == pm_selected &&
                        pm_edit.row_id == row_id &&
                        pm_edit.column == cv);
        if (editing) {
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::IsWindowAppearing() || !ImGui::IsAnyItemActive()) {
                ImGui::SetKeyboardFocusHere();
            }
            bool committed = ImGui::InputText("##edit", pm_edit.buffer,
                sizeof(pm_edit.buffer),
                ImGuiInputTextFlags_EnterReturnsTrue);
            bool escape = ImGui::IsItemDeactivated() &&
                          !ImGui::IsItemDeactivatedAfterEdit();
            if (committed) {
                commit(pm_edit.buffer);
                pm_edit.column = -1;
            } else if (escape) {
                pm_edit.column = -1;
            }
        } else {
            bool clicked = ImGui::Selectable(display.c_str(), false,
                ImGuiSelectableFlags_AllowDoubleClick);
            // Tooltip hints at where the FK points.
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s\n→ table: %s",
                                  display.c_str(),
                                  field.hint ? field.hint : "(unknown)");
            }
            if (clicked && ImGui::IsMouseDoubleClicked(0)) {
                pm_edit.dbc = pm_selected;
                pm_edit.row_id = row_id;
                pm_edit.column = cv;
                std::snprintf(pm_edit.buffer, sizeof(pm_edit.buffer),
                              "%s", val.c_str());
                pm_edit.last_error.clear();
            }
        }
        break;
    }

    // ---- Default + LocalizedString: existing double-click → InputText -----
    case DbcSemantic::LocalizedString:
    case DbcSemantic::Default:
    default:
    default_cell: {
        bool editing = (pm_edit.dbc == pm_selected &&
                        pm_edit.row_id == row_id &&
                        pm_edit.column == cv);
        if (editing) {
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::IsWindowAppearing() || !ImGui::IsAnyItemActive()) {
                ImGui::SetKeyboardFocusHere();
            }
            bool committed = ImGui::InputText("##edit", pm_edit.buffer,
                sizeof(pm_edit.buffer),
                ImGuiInputTextFlags_EnterReturnsTrue);
            bool escape = ImGui::IsItemDeactivated() &&
                          !ImGui::IsItemDeactivatedAfterEdit();
            if (committed) {
                commit(pm_edit.buffer);
                pm_edit.column = -1;
            } else if (escape) {
                pm_edit.column = -1;
            }
        } else {
            bool clicked = ImGui::Selectable(
                val.empty() ? " " : val.c_str(), false,
                ImGuiSelectableFlags_AllowDoubleClick);
            TooltipIfHovered(val.c_str());
            if (clicked && ImGui::IsMouseDoubleClicked(0)) {
                pm_edit.dbc = pm_selected;
                pm_edit.row_id = row_id;
                pm_edit.column = cv;
                std::snprintf(pm_edit.buffer, sizeof(pm_edit.buffer),
                              "%s", val.c_str());
                pm_edit.last_error.clear();
            }
        }
        break;
    }
    }
}

void DbcBrowserSystem::DrawFileTable(DbcRegistry::Entry& entry) {
    const DbcFile& dbc = *entry.file;
    const DbcSchema* schema = entry.schema;

    ImGui::SetNextItemWidth(140);
    ImGui::DragInt("Max rows", &pm_max_rows, 100.0f, 100, 100000);

    uint32_t total_rows = dbc.GetRecordCount();
    uint32_t shown = total_rows;
    if (pm_max_rows > 0 && shown > static_cast<uint32_t>(pm_max_rows)) {
        shown = static_cast<uint32_t>(pm_max_rows);
    }
    if (shown < total_rows) {
        ImGui::SameLine();
        ImGui::TextDisabled("(showing %u of %u)", shown, total_rows);
    }

    uint32_t col_count = schema ? schema->field_count : dbc.GetFieldCount();
    if (col_count == 0) {
        ImGui::TextDisabled("No columns to display.");
        return;
    }

    constexpr ImGuiTableFlags kTableFlags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

    if (!ImGui::BeginTable("##file_records", static_cast<int>(col_count), kTableFlags)) {
        return;
    }

    if (schema) {
        for (uint32_t f = 0; f < schema->field_count; f++) {
            const auto& field = schema->fields[f];
            if (IsPaddingField(field.name)) {
                ImGui::TableSetupColumn("(pad)",
                    ImGuiTableColumnFlags_DefaultHide |
                    ImGuiTableColumnFlags_WidthFixed,
                    50.0f);
            } else {
                ImGui::TableSetupColumn(field.name,
                    ImGuiTableColumnFlags_WidthFixed,
                    ColumnWidthForType(field.type));
            }
        }
    } else {
        for (uint32_t f = 0; f < col_count; f++) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "f%u", f);
            ImGui::TableSetupColumn(buf,
                ImGuiTableColumnFlags_WidthFixed,
                ColumnWidthForType(DbcFieldType::UInt32));
        }
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(shown));
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
            ImGui::TableNextRow();
            for (uint32_t f = 0; f < col_count; f++) {
                ImGui::TableSetColumnIndex(static_cast<int>(f));
                if (schema) {
                    if (schema->packed) {
                        DrawPackedFileCell(dbc, schema, static_cast<uint32_t>(row), f);
                    } else {
                        DrawNonPackedFileCell(dbc, schema->fields[f],
                                              static_cast<uint32_t>(row), f);
                    }
                } else {
                    ImGui::Text("%u",
                                dbc.GetUInt32(static_cast<uint32_t>(row), f));
                }
            }
        }
    }
    ImGui::EndTable();
}

} // namespace mve
