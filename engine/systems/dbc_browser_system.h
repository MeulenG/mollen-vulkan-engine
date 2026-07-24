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
};

} // namespace mve

#endif // MVE_DBC_BROWSER_SYSTEM_H
