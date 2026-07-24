#include "dbc_browser_system.h"

#include <imgui.h>

#include <cstring>
#include <cctype>

namespace mve {

namespace {

bool IsPaddingField(const char* name) {
    return name && name[0] == '_' && std::strncmp(name, "_pad", 4) == 0;
}

bool MatchesFilter(const std::string& name, const char* filter) {
    if (!filter || !*filter) return true;
    // Case-insensitive substring match.
    auto to_lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::string n; n.reserve(name.size());
    for (unsigned char c : name) n.push_back(to_lower(c));
    std::string f; f.reserve(std::strlen(filter));
    for (const char* p = filter; *p; ++p) f.push_back(to_lower(static_cast<unsigned char>(*p)));
    return n.find(f) != std::string::npos;
}

// Render a single cell value from a non-packed (4-byte aligned) DBC.
void DrawNonPackedCell(const DbcFile& dbc, const DbcFieldDef& field,
                       uint32_t record, uint32_t field_index) {
    switch (field.type) {
    case DbcFieldType::UInt32:
    case DbcFieldType::UInt8:
    case DbcFieldType::UInt16:
        ImGui::Text("%u", dbc.GetUInt32(record, field_index));
        break;
    case DbcFieldType::Int32:
    case DbcFieldType::Int8:
    case DbcFieldType::Int16:
        ImGui::Text("%d", dbc.GetInt32(record, field_index));
        break;
    case DbcFieldType::Float:
        ImGui::Text("%.3f", dbc.GetFloat(record, field_index));
        break;
    case DbcFieldType::String: {
        const char* s = dbc.GetStringField(record, field_index);
        ImGui::TextUnformatted(s ? s : "");
        break;
    }
    }
}

void DrawPackedCell(const DbcFile& dbc, const DbcSchema* schema,
                    uint32_t record, uint32_t field_index) {
    uint32_t offset = DbcFile::GetFieldOffset(schema, field_index);
    const auto& field = schema->fields[field_index];
    switch (field.type) {
    case DbcFieldType::UInt8:
        ImGui::Text("%u", dbc.GetUInt8At(record, offset));
        break;
    case DbcFieldType::Int8:
        ImGui::Text("%d", dbc.GetInt8At(record, offset));
        break;
    case DbcFieldType::UInt16:
        ImGui::Text("%u", dbc.GetUInt16At(record, offset));
        break;
    case DbcFieldType::Int16:
        ImGui::Text("%d", dbc.GetInt16At(record, offset));
        break;
    case DbcFieldType::UInt32:
        ImGui::Text("%u", dbc.GetUInt32At(record, offset));
        break;
    case DbcFieldType::Int32:
        ImGui::Text("%d", dbc.GetInt32At(record, offset));
        break;
    case DbcFieldType::Float:
        ImGui::Text("%.3f", dbc.GetFloatAt(record, offset));
        break;
    case DbcFieldType::String: {
        const char* s = dbc.GetStringAt(record, offset);
        ImGui::TextUnformatted(s ? s : "");
        break;
    }
    }
}

} // namespace

DbcBrowserSystem::DbcBrowserSystem(DbcRegistry& registry)
    : pm_registry{registry} {}

void DbcBrowserSystem::Update() {
    ImGui::Begin("DBC Browser");

    const auto& names = pm_registry.AvailableNames();
    if (names.empty()) {
        ImGui::TextWrapped("No .dbc files found in: %s",
                           pm_registry.DbcDir().string().c_str());
        ImGui::TextWrapped("Run dbc-extract --export to populate this directory.");
        ImGui::End();
        return;
    }

    // Two-pane layout via child regions
    float list_width = 260.0f;
    ImGui::BeginChild("DbcList", ImVec2(list_width, 0), ImGuiChildFlags_Borders);
    DrawDbcList();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("DbcTable", ImVec2(0, 0), ImGuiChildFlags_Borders);
    DrawRecordTable();
    ImGui::EndChild();

    ImGui::End();
}

void DbcBrowserSystem::DrawDbcList() {
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##filter", "Filter...", pm_filter, sizeof(pm_filter));
    ImGui::Separator();

    for (const auto& name : pm_registry.AvailableNames()) {
        if (!MatchesFilter(name, pm_filter)) continue;

        bool selected = (name == pm_selected);
        bool has_schema = pm_registry.HasSchema(name);

        if (!has_schema) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        if (ImGui::Selectable(name.c_str(), selected)) {
            pm_selected = name;
        }
        if (!has_schema) {
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("No schema registered (raw view only)");
            }
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
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Entry not found.");
        return;
    }
    if (entry->load_failed || !entry->file) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
                           "Failed to load %s", entry->path.string().c_str());
        return;
    }

    const DbcFile& dbc = *entry->file;
    const DbcSchema* schema = entry->schema;

    ImGui::Text("File:    %s", entry->path.filename().string().c_str());
    ImGui::Text("Records: %u", dbc.GetRecordCount());
    ImGui::Text("Fields:  %u  (record size %u bytes)",
                dbc.GetFieldCount(), dbc.GetRecordSize());

    if (!schema) {
        ImGui::TextWrapped("No schema registered for this DBC. "
                           "Showing raw 4-byte columns as uint32.");
    }

    ImGui::Separator();
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

    // Column count: visible fields from schema, or raw uint32 columns
    uint32_t col_count = schema ? schema->field_count : dbc.GetFieldCount();
    if (col_count == 0) {
        ImGui::TextDisabled("No columns to display.");
        return;
    }

    constexpr ImGuiTableFlags kTableFlags =
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingFixedFit;

    if (ImGui::BeginTable("##records", static_cast<int>(col_count), kTableFlags)) {
        // Header
        if (schema) {
            for (uint32_t f = 0; f < schema->field_count; f++) {
                const char* name = schema->fields[f].name;
                if (IsPaddingField(name)) {
                    ImGui::TableSetupColumn("(pad)", ImGuiTableColumnFlags_DefaultHide);
                } else {
                    ImGui::TableSetupColumn(name);
                }
            }
        } else {
            for (uint32_t f = 0; f < col_count; f++) {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "f%u", f);
                ImGui::TableSetupColumn(buf);
            }
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Rows — clip to viewport for performance
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(shown));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                ImGui::TableNextRow();
                for (uint32_t f = 0; f < col_count; f++) {
                    ImGui::TableSetColumnIndex(static_cast<int>(f));
                    if (schema) {
                        if (schema->packed) {
                            DrawPackedCell(dbc, schema,
                                           static_cast<uint32_t>(row), f);
                        } else {
                            DrawNonPackedCell(dbc, schema->fields[f],
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
}

} // namespace mve
