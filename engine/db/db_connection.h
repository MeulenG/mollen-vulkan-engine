#ifndef MVE_DB_CONNECTION_H
#define MVE_DB_CONNECTION_H

#include "dbc_schema.h"

#include <string>
#include <unordered_set>
#include <vector>
#include <cstdint>

// Forward-declare so we don't drag pqxx into every translation unit.
class psql_connector;

namespace mve {

// Engine-side facade over the dbc/psql/psql_connector. Owns the connection,
// caches the set of existing public-schema tables, and exposes the small
// number of operations the editor actually performs:
//
//   - read all rows of a table     (FetchTable)
//   - re-read a single row         (FetchRow)
//   - update one cell              (UpdateCell)
//
// All methods are safe to call when not connected - they fail and write a
// human-readable reason into LastError().
//
// String values are the universal currency: every cell is rendered as text in
// ImGui and every SQL bind goes via pqxx exec_params (typed at the schema
// level, not the C++ level). This keeps the surface area small.
class DbConnection {
public:
    struct Row {
        // One entry per column, in the same order as Table::columns.
        // NULLs become empty strings - DBC schemas don't use NULL.
        std::vector<std::string> values;
    };

    struct Table {
        std::vector<std::string> columns;  // snake_case, matches dbc_db_import
        std::vector<Row> rows;
    };

    DbConnection();
    ~DbConnection();

    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;

    // Read db_config.toml at `toml_path` and connect. Returns true on success.
    // On failure, LastError() explains why.
    bool Connect(const std::string& toml_path);

    void Disconnect();
    bool IsConnected() const;

    // Display helpers (empty when not connected)
    const std::string& DbName() const { return pm_dbname; }
    const std::string& Host() const { return pm_host; }
    int Port() const { return pm_port; }
    const std::string& LastError() const { return pm_last_error; }

    // True iff `table` (lowercase) exists in the public schema. Backed by a
    // cache populated at connect time; call RefreshSchema() to update.
    bool TableExists(const std::string& table) const;
    void RefreshSchema();

    // Fetch every row of a table. Rows are returned in primary-key order.
    bool FetchTable(const std::string& table, Table& out);

    // Re-fetch one row by primary key. id_column is typically "id".
    bool FetchRow(const std::string& table,
                  const std::string& id_column,
                  int64_t id,
                  Row& out);

    // Update one cell. The SQL cast is chosen from `type` so the bind matches
    // the column. value is the literal text the user typed.
    //
    //   UPDATE "<table>" SET "<column>" = $1::<sql_type> WHERE "<id_column>" = $2
    bool UpdateCell(const std::string& table,
                    const std::string& id_column,
                    int64_t id,
                    const std::string& column,
                    const std::string& value,
                    DbcFieldType type);

private:
    // Owned connector - pImpl-style so the header doesn't include pqxx.
    psql_connector* pm_conn = nullptr;
    bool pm_connected = false;

    std::unordered_set<std::string> pm_known_tables;

    std::string pm_dbname;
    std::string pm_host;
    int pm_port = 0;
    std::string pm_last_error;
};

} // namespace mve

#endif // MVE_DB_CONNECTION_H
