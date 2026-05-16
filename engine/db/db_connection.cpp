#include "db_connection.h"

#ifdef MVE_ENABLE_PSQL

#include "psql_connector.h"
#include "db_config.h"

#include <pqxx/pqxx>

namespace mve {

namespace {

// Map a DbcFieldType to the PostgreSQL cast we use in UPDATE statements.
// The column itself is already typed in the schema (BIGINT/INTEGER/REAL/TEXT),
// but casting the bind parameter avoids round-tripping the literal as TEXT
// and getting "operator does not exist" errors.
const char* SqlCast(DbcFieldType type) {
    switch (type) {
    case DbcFieldType::UInt32: return "bigint";
    case DbcFieldType::Int32:
    case DbcFieldType::UInt8:
    case DbcFieldType::Int8:
    case DbcFieldType::UInt16:
    case DbcFieldType::Int16:  return "integer";
    case DbcFieldType::Float:  return "real";
    case DbcFieldType::String: return "text";
    }
    return "text";
}

} // namespace

DbConnection::DbConnection()
    : pm_conn{new psql_connector{}} {}

DbConnection::~DbConnection() {
    Disconnect();
    delete pm_conn;
}

bool DbConnection::Connect(const std::string& toml_path) {
    pm_last_error.clear();

    DbConfig cfg;
    if (!LoadDbConfig(toml_path, cfg)) {
        pm_last_error = "Failed to read " + toml_path;
        return false;
    }

    if (!pm_conn->Connect(cfg)) {
        pm_last_error = "Connection failed (see stderr for SQLSTATE)";
        return false;
    }

    pm_dbname = cfg.dbname;
    pm_host = cfg.host;
    pm_port = cfg.port;
    pm_connected = true;

    RefreshSchema();
    return true;
}

void DbConnection::Disconnect() {
    if (pm_connected && pm_conn) {
        pm_conn->Disconnect();
    }
    pm_connected = false;
    pm_known_tables.clear();
    pm_dbname.clear();
    pm_host.clear();
    pm_port = 0;
}

bool DbConnection::IsConnected() const {
    return pm_connected && pm_conn && pm_conn->IsConnected();
}

bool DbConnection::TableExists(const std::string& table) const {
    return pm_known_tables.count(table) > 0;
}

void DbConnection::RefreshSchema() {
    pm_known_tables.clear();
    if (!IsConnected()) return;

    try {
        pqxx::work txn{pm_conn->GetConnection()};
        auto result = txn.exec(
            "SELECT tablename FROM pg_catalog.pg_tables "
            "WHERE schemaname = 'public'");
        for (const auto& row : result) {
            pm_known_tables.insert(row[0].as<std::string>());
        }
        txn.commit();
    } catch (const std::exception& e) {
        pm_last_error = std::string{"RefreshSchema: "} + e.what();
    }
}

bool DbConnection::FetchTable(const std::string& table, Table& out) {
    out.columns.clear();
    out.rows.clear();
    if (!IsConnected()) {
        pm_last_error = "Not connected";
        return false;
    }

    try {
        pqxx::work txn{pm_conn->GetConnection()};

        // Quote the identifier ourselves - pqxx exec_params doesn't bind
        // identifiers, only values. Caller is responsible for passing a
        // table name that came from DbcTableName() (lowercase, no quotes).
        std::string sql = "SELECT * FROM \"" + table + "\"";
        // Order by primary key when present so the UI is stable across reloads.
        sql += " ORDER BY 1";

        auto result = txn.exec(sql);

        out.columns.reserve(result.columns());
        for (int c = 0; c < static_cast<int>(result.columns()); c++) {
            out.columns.emplace_back(result.column_name(c));
        }

        out.rows.reserve(result.size());
        for (const auto& row : result) {
            Row r;
            r.values.reserve(out.columns.size());
            for (int c = 0; c < static_cast<int>(row.size()); c++) {
                r.values.emplace_back(row[c].is_null() ? "" : row[c].as<std::string>());
            }
            out.rows.push_back(std::move(r));
        }

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        pm_last_error = std::string{"FetchTable("} + table + "): " + e.what();
        return false;
    }
}

bool DbConnection::FetchRow(const std::string& table,
                            const std::string& id_column,
                            int64_t id,
                            Row& out) {
    out.values.clear();
    if (!IsConnected()) {
        pm_last_error = "Not connected";
        return false;
    }

    try {
        pqxx::work txn{pm_conn->GetConnection()};
        std::string sql = "SELECT * FROM \"" + table + "\" WHERE \"" + id_column + "\" = $1";
        auto result = txn.exec_params(sql, id);

        if (result.empty()) {
            pm_last_error = "Row not found";
            txn.commit();
            return false;
        }

        const auto& row = result[0];
        out.values.reserve(row.size());
        for (int c = 0; c < static_cast<int>(row.size()); c++) {
            out.values.emplace_back(row[c].is_null() ? "" : row[c].as<std::string>());
        }
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        pm_last_error = std::string{"FetchRow: "} + e.what();
        return false;
    }
}

bool DbConnection::UpdateCell(const std::string& table,
                              const std::string& id_column,
                              int64_t id,
                              const std::string& column,
                              const std::string& value,
                              DbcFieldType type) {
    if (!IsConnected()) {
        pm_last_error = "Not connected";
        return false;
    }

    try {
        pqxx::work txn{pm_conn->GetConnection()};

        std::string sql =
            "UPDATE \"" + table + "\" "
            "SET \"" + column + "\" = $1::" + SqlCast(type) + " "
            "WHERE \"" + id_column + "\" = $2";

        // We pass the value as TEXT and cast on the server side. PostgreSQL
        // rejects malformed numerics with a clean error message, which beats
        // doing the parse in C++ and surfacing locale-specific atoi errors.
        txn.exec_params(sql, value, id);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        pm_last_error = std::string{"UpdateCell: "} + e.what();
        return false;
    }
}

} // namespace mve

#else // MVE_ENABLE_PSQL

// Stubs for builds without libpqxx. Everything fails politely.

namespace mve {

DbConnection::DbConnection() = default;
DbConnection::~DbConnection() = default;

bool DbConnection::Connect(const std::string& /*toml_path*/) {
    pm_last_error = "Built without MVE_ENABLE_PSQL";
    return false;
}
void DbConnection::Disconnect() {}
bool DbConnection::IsConnected() const { return false; }
bool DbConnection::TableExists(const std::string& /*t*/) const { return false; }
void DbConnection::RefreshSchema() {}
bool DbConnection::FetchTable(const std::string&, Table&) {
    pm_last_error = "Built without MVE_ENABLE_PSQL";
    return false;
}
bool DbConnection::FetchRow(const std::string&, const std::string&, int64_t, Row&) {
    pm_last_error = "Built without MVE_ENABLE_PSQL";
    return false;
}
bool DbConnection::UpdateCell(const std::string&, const std::string&, int64_t,
                              const std::string&, const std::string&, DbcFieldType) {
    pm_last_error = "Built without MVE_ENABLE_PSQL";
    return false;
}

} // namespace mve

#endif // MVE_ENABLE_PSQL
