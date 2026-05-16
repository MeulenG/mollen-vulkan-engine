#include "dbc_form_system.h"
#include "dbc_naming.h"

#include <imgui.h>

#include <cstdio>
#include <cstring>
#include <unordered_set>

namespace mve {

namespace {

bool IsPaddingField(const char* name) {
    return name && name[0] == '_' && std::strncmp(name, "_pad", 4) == 0;
}

const char* SemanticLabel(DbcSemantic s) {
    switch (s) {
    case DbcSemantic::Boolean:         return "bool";
    case DbcSemantic::Enum:            return "enum";
    case DbcSemantic::ForeignKey:      return "fk";
    case DbcSemantic::Color:           return "color";
    case DbcSemantic::Bitmask:         return "bitmask";
    case DbcSemantic::LocalizedString: return "i18n";
    case DbcSemantic::Default:
    default:                           return "";
    }
}

const char* kFallbackCategory = "Misc";

} // namespace

DbcFormSystem::DbcFormSystem(DbcRegistry& registry, DbConnection& db)
    : pm_registry{registry}, pm_db{db} {}

void DbcFormSystem::Open(const std::string& dbc_name, int64_t row_id) {
    pm_dbc_name = dbc_name;
    pm_row_id = row_id;
    pm_needs_refresh = true;
    pm_last_error.clear();
}

void DbcFormSystem::Close() {
    pm_dbc_name.clear();
    pm_row_id = 0;
    pm_row.values.clear();
    pm_columns.clear();
    pm_edit_buffers.clear();
}

void DbcFormSystem::RefreshRow() {
    pm_needs_refresh = false;

    auto* entry = pm_registry.Load(pm_dbc_name);
    if (!entry) {
        pm_last_error = "DBC not in registry: " + pm_dbc_name;
        return;
    }

    std::string table = DbcTableName(pm_dbc_name.c_str());

    // We need both the column ordering and the row values. FetchRow gives
    // us values but the row alone doesn't expose its columns — fetch the
    // whole table descriptor once to learn columns, then just the one row.
    DbConnection::Table tbl;
    if (!pm_db.FetchTable(table, tbl)) {
        pm_last_error = "Failed to fetch " + table + ": " + pm_db.LastError();
        return;
    }
    pm_columns = tbl.columns;

    DbConnection::Row fresh;
    if (!pm_db.FetchRow(table, "id", pm_row_id, fresh)) {
        pm_last_error = "Row " + std::to_string(pm_row_id) +
                        " not found: " + pm_db.LastError();
        pm_row.values.clear();
        return;
    }
    pm_row = std::move(fresh);

    // Reset edit buffers to current values.
    pm_edit_buffers.assign(pm_row.values.begin(), pm_row.values.end());
    pm_last_error.clear();
}

void DbcFormSystem::Update() {
    // Always begin the window so docking layouts stay stable. When closed
    // we just show a friendly placeholder.
    ImGui::Begin("Form");

    if (!IsOpen()) {
        ImGui::TextDisabled("No row open.\n\nUse the DBC Browser's \"Open in form\" "
                            "button on a selected row to load it here.");
        ImGui::End();
        return;
    }

    if (pm_needs_refresh) RefreshRow();

    auto* entry = pm_registry.Load(pm_dbc_name);
    const DbcSchema* schema = entry ? entry->schema : nullptr;
    if (!schema) {
        ImGui::Text("No schema for %s", pm_dbc_name.c_str());
        if (ImGui::Button("Close")) Close();
        ImGui::End();
        return;
    }

    DrawHeader(schema);
    ImGui::Separator();

    if (!pm_last_error.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", pm_last_error.c_str());
        ImGui::Separator();
    }

    // Collect distinct categories in field order — same approach as the
    // browser's section toggles. Stable order matters for muscle memory.
    std::vector<const char*> categories;
    std::unordered_set<std::string> seen;
    bool has_uncategorized = false;
    for (uint32_t f = 0; f < schema->field_count; f++) {
        const char* c = schema->fields[f].category;
        if (!c || !*c) { has_uncategorized = true; continue; }
        if (seen.insert(c).second) categories.push_back(c);
    }
    if (has_uncategorized) categories.push_back(kFallbackCategory);

    for (const char* category : categories) {
        DrawSection(schema, category);
    }

    ImGui::End();
}

void DbcFormSystem::DrawHeader(const DbcSchema* schema) {
    std::string pretty = DbcPrettyName(pm_dbc_name.c_str());
    ImGui::Text("%s  #%lld", pretty.c_str(), static_cast<long long>(pm_row_id));

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(8, 0));
    ImGui::SameLine();
    ImGui::TextDisabled("schema: %s", schema->dbc_name);

    if (ImGui::SmallButton("Refresh")) pm_needs_refresh = true;
    ImGui::SameLine();
    if (ImGui::SmallButton("Close")) Close();
}

void DbcFormSystem::DrawSection(const DbcSchema* schema, const char* category) {
    // Default-open Identity / Cast / first-effect-style sections; everything
    // else starts collapsed. Picks "important" by name — cheap heuristic.
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
    if (!std::strcmp(category, "Identity") ||
        !std::strcmp(category, "Cast")     ||
        !std::strcmp(category, "Effects")) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (!ImGui::CollapsingHeader(category, flags)) return;

    // Two-column layout: label on the left, value editor on the right.
    if (!ImGui::BeginTable(category, 2,
                           ImGuiTableFlags_SizingStretchProp |
                           ImGuiTableFlags_PadOuterX |
                           ImGuiTableFlags_BordersInnerH)) {
        return;
    }
    ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 220);
    ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);

    for (uint32_t f = 0; f < schema->field_count; f++) {
        const auto& field = schema->fields[f];
        if (IsPaddingField(field.name)) continue;

        const char* fc = field.category && *field.category
                             ? field.category : kFallbackCategory;
        if (std::strcmp(fc, category) != 0) continue;

        DrawFieldRow(schema, f);
    }

    ImGui::EndTable();
}

bool DbcFormSystem::DrawFieldRow(const DbcSchema* schema,
                                 uint32_t field_index) {
    const auto& field = schema->fields[field_index];

    // Find db column.
    std::string col_name = DbcColumnName(field.name);
    int db_col = -1;
    for (size_t i = 0; i < pm_columns.size(); i++) {
        if (pm_columns[i] == col_name) { db_col = static_cast<int>(i); break; }
    }
    if (db_col < 0 || db_col >= static_cast<int>(pm_row.values.size())) {
        return false;
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    // Label cell. Show field name; muted tag for non-default semantics so
    // the user has a hint about how the value will eventually be edited.
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(field.name);
    const char* tag = SemanticLabel(field.semantic);
    if (*tag) {
        ImGui::SameLine();
        ImGui::TextDisabled("[%s]", tag);
    }

    ImGui::TableSetColumnIndex(1);

    // v1 value editor: a single InputText that commits on focus loss.
    // Future enhancement: per-semantic widgets (combo, checkbox, etc.)
    // — same dispatch logic as the browser's table cells.
    constexpr size_t kBufCap = 1024;
    std::string& buf = pm_edit_buffers[db_col];
    if (buf.capacity() < kBufCap) buf.reserve(kBufCap);

    ImGui::PushID(static_cast<int>(field_index));
    ImGui::SetNextItemWidth(-FLT_MIN);

    bool field_is_string = (field.type == DbcFieldType::String);
    if (field_is_string && buf.size() > 60) {
        // Long-form text gets multiline. Standard for Description/ToolTip.
        ImGui::InputTextMultiline(
            "##v", buf.data(), buf.capacity(),
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
    } else {
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
    }

    bool committed = false;
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        std::string current = buf.c_str();  // strip trailing NULs
        Commit(col_name, current, field.type);
        committed = true;
    }
    ImGui::PopID();

    return committed;
}

void DbcFormSystem::Commit(const std::string& column,
                           const std::string& value,
                           DbcFieldType type) {
    if (!pm_db.UpdateCell(DbcTableName(pm_dbc_name.c_str()),
                          "id", pm_row_id, column, value, type)) {
        pm_last_error = pm_db.LastError();
        return;
    }
    pm_last_error.clear();
    pm_needs_refresh = true;  // refetch row to pick up any server-side normalization
}

} // namespace mve
