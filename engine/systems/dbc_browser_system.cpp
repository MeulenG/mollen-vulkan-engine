#include "dbc_browser_system.h"
#include "dbc_naming.h"
#include "schema_registry.h"

#include <imgui.h>

#include <cstring>
#include <cctype>
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
                                ColumnWidthForType(field.type));
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

                const std::string& val = db_row.values[v.db_column];
                bool editing = (pm_edit.dbc == pm_selected &&
                                pm_edit.row_id == row_id &&
                                pm_edit.column == static_cast<int>(cv));

                ImGui::PushID(row);
                ImGui::PushID(static_cast<int>(cv));

                if (editing) {
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    if (ImGui::IsWindowAppearing() ||
                        !ImGui::IsAnyItemActive()) {
                        ImGui::SetKeyboardFocusHere();
                    }
                    bool committed = ImGui::InputText(
                        "##edit", pm_edit.buffer, sizeof(pm_edit.buffer),
                        ImGuiInputTextFlags_EnterReturnsTrue);
                    bool escape = ImGui::IsItemDeactivated() &&
                                  !ImGui::IsItemDeactivatedAfterEdit();

                    if (committed) {
                        // Send UPDATE.
                        std::string col = v.col_name;
                        bool ok = pm_db.UpdateCell(
                            DbcTableName(pm_selected.c_str()),
                            "id", row_id, col,
                            pm_edit.buffer,
                            schema->fields[v.field_index].type);
                        if (ok) {
                            // Refresh just this row, fall back to full refetch
                            // if for some reason the row vanished.
                            DbConnection::Row fresh;
                            if (pm_db.FetchRow(
                                    DbcTableName(pm_selected.c_str()),
                                    "id", row_id, fresh) &&
                                fresh.values.size() == table.rows[row].values.size()) {
                                table.rows[row] = std::move(fresh);
                            } else {
                                InvalidateTable(pm_selected);
                            }
                            pm_edit.last_error.clear();
                            pm_edit.column = -1;
                        } else {
                            pm_edit.last_error = pm_db.LastError();
                        }
                    } else if (escape) {
                        pm_edit.column = -1;
                        pm_edit.last_error.clear();
                    }
                } else {
                    // Display mode. Right-click context menu would be nice
                    // later; for now, click selects, double-click edits.
                    bool clicked = ImGui::Selectable(
                        val.empty() ? " " : val.c_str(),
                        false,
                        ImGuiSelectableFlags_AllowDoubleClick);
                    TooltipIfHovered(val.c_str());
                    if (clicked && ImGui::IsMouseDoubleClicked(0)) {
                        pm_edit.dbc = pm_selected;
                        pm_edit.row_id = row_id;
                        pm_edit.column = static_cast<int>(cv);
                        std::snprintf(pm_edit.buffer, sizeof(pm_edit.buffer),
                                      "%s", val.c_str());
                        pm_edit.last_error.clear();
                    }
                }

                ImGui::PopID();
                ImGui::PopID();
            }
        }
    }
    ImGui::EndTable();
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
