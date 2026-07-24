#ifndef MVE_DBC_FORM_SYSTEM_H
#define MVE_DBC_FORM_SYSTEM_H

#include "../resources/dbc_registry.h"
#include "../db/db_connection.h"

#include <string>
#include <vector>

namespace mve {

// One-row form editor for a DBC. Complementary to DbcBrowserSystem's table
// view — the table is for browsing, this is for deep edit of a single row
// with sections grouped by `DbcFieldDef::category` and one field per row.
//
// Open via DbcBrowserSystem's "Open in form" button. The form fetches the
// row on open and re-fetches after each commit so values stay consistent
// with the DB.
//
// v1 scope: render every field with a simple label + value editor that
// commits on focus loss / Enter. Semantic-specific widgets (combos, color
// pickers, etc.) are deferred to a polish pass — the form is already a
// major UX upgrade for Spell.dbc just by virtue of the section grouping.
class DbcFormSystem {
public:
    DbcFormSystem(DbcRegistry& registry, DbConnection& db);

    void Update();

    // Called by the browser when the user requests "open this row in form".
    void Open(const std::string& dbc_name, int64_t row_id);
    void Close();
    bool IsOpen() const { return !pm_dbc_name.empty(); }

private:
    void RefreshRow();
    void DrawHeader(const DbcSchema* schema);
    void DrawSection(const DbcSchema* schema, const char* category);

    // Draws one field-row inside a section. Renders the label and a value
    // editor sized to fit the form's right column. Returns true if the user
    // committed an edit (caller usually doesn't care — Commit already ran).
    bool DrawFieldRow(const DbcSchema* schema, uint32_t field_index);

    // Commits the cell via DbConnection and refreshes the cached row on success.
    void Commit(const std::string& column,
                const std::string& value,
                DbcFieldType type);

    DbcRegistry& pm_registry;
    DbConnection& pm_db;

    std::string pm_dbc_name;
    int64_t pm_row_id = 0;
    DbConnection::Row pm_row;
    std::vector<std::string> pm_columns;  // column names matching pm_row.values
    bool pm_needs_refresh = false;
    std::string pm_last_error;

    // Per-field edit buffer for committed-on-enter text inputs. Keyed by
    // (column name) since field names alone collide across schemas.
    // For form view this lives only while the form is open for one row.
    std::vector<std::string> pm_edit_buffers;  // parallel to pm_row.values
};

} // namespace mve

#endif // MVE_DBC_FORM_SYSTEM_H
