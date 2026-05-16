#ifndef MVE_DBC_BROWSER_SYSTEM_H
#define MVE_DBC_BROWSER_SYSTEM_H

#include "../resources/dbc_registry.h"
#include "../db/db_connection.h"

#include <string>
#include <unordered_map>

namespace mve {

// ImGui panel for browsing and editing DBC data.
//
// Two source modes, chosen per-DBC at display time:
//
//   - PSQL mode    — DbConnection is connected AND the table exists in the
//                    public schema. Rows come from SELECT, cells are
//                    editable, edits commit via UPDATE.
//   - file mode    — fallback for any other case (no connection, table
//                    missing, schema mismatch, no schema registered).
//                    Rows come from the .dbc file via DbcRegistry, cells
//                    are read-only.
//
// PSQL tables are fetched once and cached. The user can refresh per-table
// or after a failed UPDATE.
class DbcBrowserSystem {
public:
    DbcBrowserSystem(DbcRegistry& registry, DbConnection& db);

    void Update();

private:
    enum class SourceMode { File, Psql };

    struct EditState {
        std::string dbc;       // selected dbc name when entering edit
        int64_t row_id = 0;    // primary key value
        int column = -1;       // -1 = no edit in progress
        char buffer[256] = {0};
        std::string last_error;
    };

    void DrawConnectionHeader();
    void DrawDbcList();
    void DrawRecordTable();

    // PSQL helpers
    SourceMode SourceFor(const std::string& name) const;
    DbConnection::Table* GetCachedTable(const std::string& dbc_name);
    void InvalidateTable(const std::string& dbc_name);

    void DrawPsqlTable(DbcRegistry::Entry& entry,
                       DbConnection::Table& table);
    void DrawFileTable(DbcRegistry::Entry& entry);

    // Tabbed popup for editing every locale of a LocalizedString cluster.
    // Reads + commits each tab as a separate UPDATE against its own column.
    void DrawLocaleEditPopup(DbConnection::Table& table);

    // Per-cell semantic dispatch. Each renders the appropriate widget for
    // the field's DbcSemantic and, on user change, commits via UpdateCell
    // and refreshes the cached row in `table`.
    //
    // `cv` is the index into the visible-column view for ImGui ID scoping;
    // we don't need to pass it around for SQL purposes.
    void DrawPsqlCell(const DbcSchema* schema,
                      int field_index,
                      DbConnection::Table& table,
                      int row, int row_id, int db_column,
                      int cv);

    DbcRegistry& pm_registry;
    DbConnection& pm_db;

    std::string pm_selected;
    char pm_filter[64] = {0};
    int pm_max_rows = 1000;

    // Cache of fetched PSQL tables, keyed by DBC name (NOT lowercased table
    // name — keeps lookups consistent with the registry).
    std::unordered_map<std::string, DbConnection::Table> pm_psql_cache;

    // Foreign-key label cache. Keyed by target SQL table name (lowercase).
    // Built lazily on first FK render that points at a given table; survives
    // for the lifetime of the connection (invalidated on RefreshSchema).
    struct FkLabelCache {
        std::unordered_map<int64_t, std::string> id_to_label;
        bool resolved = false;  // true once we attempted to populate it
    };
    std::unordered_map<std::string, FkLabelCache> pm_fk_cache;
    const std::string& ResolveFkLabel(const std::string& target_table, int64_t id);

    EditState pm_edit;

    // ---- Section visibility (Layer 2: categories) ----
    //
    // Per-DBC, per-category visibility map. Built lazily on first render of
    // a DBC; categories absent from the inner map default to visible. We
    // store per-DBC so toggling categories on Spell doesn't affect Item.
    std::unordered_map<std::string, std::unordered_map<std::string, bool>>
        pm_section_visible;

    // Draws the section toggle checkbox bar for the currently-selected DBC's
    // distinct categories. Returns the set of visible categories so the
    // caller can filter the column view.
    void DrawSectionToggles(const DbcSchema* schema);
    bool IsCategoryVisible(const char* category) const;

    // ---- Locale cluster editing ----
    //
    // A DBC like ChrRaces has 16 separate `Name_enUS`, `Name_koKR`, ... columns
    // that the auto-tagger marked with `DbcSemantic::LocalizedString` and a
    // common hint ("Name"). We collapse those into a single visible column
    // showing the enUS value; double-click opens this popup with one tab per
    // locale. Edits go through the standard UpdateCell path, one column at a
    // time, so multi-locale updates aren't atomic but each commit is.
    struct LocaleEditState {
        std::string dbc;
        int64_t row_id = 0;
        std::string hint;          // e.g. "Name" — shared across cluster members

        // Parallel arrays: one entry per locale, matching the schema field order.
        std::vector<std::string> field_names;   // e.g. "Name_enUS"
        std::vector<std::string> col_names;     // matching DB column (snake_case)
        std::vector<std::string> buffers;       // editable working values

        std::string last_error;
        bool just_opened = false;  // request focus on the enUS tab on open
    };
    LocaleEditState pm_locale_edit;
};

} // namespace mve

#endif // MVE_DBC_BROWSER_SYSTEM_H
