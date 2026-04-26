#include "dbc_db_import.h"
#include "dbc_naming.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>

static bool IsPadding(const char* name) {
    return name && name[0] == '_' && strncmp(name, "_pad", 4) == 0;
}

static std::string PgTypeName(DbcFieldType type) {
    switch (type) {
    case DbcFieldType::UInt8:
    case DbcFieldType::Int8:
    case DbcFieldType::UInt16:
    case DbcFieldType::Int16:
        return "INTEGER";
    case DbcFieldType::UInt32:
        return "BIGINT";
    case DbcFieldType::Int32:
        return "INTEGER";
    case DbcFieldType::Float:
        return "REAL";
    case DbcFieldType::String:
        return "TEXT";
    }
    return "BIGINT";
}

static std::string EscapeSql(const char* str) {
    if (!str) return "";
    std::string result;
    for (const char* p = str; *p; ++p) {
        if (*p == '\'') {
            result += "''";
        } else if ((unsigned char)*p < 0x20 && *p != '\n' && *p != '\r' && *p != '\t') {
            // Skip non-printable control characters
        } else {
            result += *p;
        }
    }
    return result;
}

static std::string QuoteIdent(const std::string& name) {
    return "\"" + name + "\"";
}

bool DbCreateTable(psql_connector& db, const DbcSchema* schema) {
    if (!schema || !db.IsConnected()) return false;

    std::string table_name = DbcTableName(schema->dbc_name);

    std::ostringstream sql;
    sql << "CREATE TABLE IF NOT EXISTS " << QuoteIdent(table_name) << " (\n";

    bool first = true;
    bool has_id = false;
    for (uint32_t f = 0; f < schema->field_count; f++) {
        if (IsPadding(schema->fields[f].name)) continue;

        if (!first) sql << ",\n";
        first = false;

        std::string col = DbcColumnName(schema->fields[f].name);
        sql << "    " << QuoteIdent(col) << " " << PgTypeName(schema->fields[f].type);

        if (f == 0 && strcmp(schema->fields[f].name, "Id") == 0) {
            sql << " PRIMARY KEY";
            has_id = true;
        }
    }

    sql << "\n)";

    try {
        db.Exec("DROP TABLE IF EXISTS " + QuoteIdent(table_name));
        db.Exec(sql.str());
        return true;
    } catch (const std::exception& e) {
        fprintf(stderr, "Failed to create table %s: %s\n", table_name.c_str(), e.what());
        return false;
    }
}

static std::string GetFieldValueSql(const DbcFile& dbc, const DbcSchema* schema,
                                    uint32_t record, uint32_t field) {
    if (schema->packed) {
        uint32_t offset = DbcFile::GetFieldOffset(schema, field);
        switch (schema->fields[field].type) {
        case DbcFieldType::UInt8:
            return std::to_string(dbc.GetUInt8At(record, offset));
        case DbcFieldType::Int8:
            return std::to_string(dbc.GetInt8At(record, offset));
        case DbcFieldType::UInt16:
            return std::to_string(dbc.GetUInt16At(record, offset));
        case DbcFieldType::Int16:
            return std::to_string(dbc.GetInt16At(record, offset));
        case DbcFieldType::UInt32:
            return std::to_string(dbc.GetUInt32At(record, offset));
        case DbcFieldType::Int32:
            return std::to_string(dbc.GetInt32At(record, offset));
        case DbcFieldType::Float: {
            float val = dbc.GetFloatAt(record, offset);
            if (std::isnan(val) || std::isinf(val)) return "0";
            return std::to_string(val);
        }
        case DbcFieldType::String:
            return "'" + EscapeSql(dbc.GetStringAt(record, offset)) + "'";
        }
    } else {
        switch (schema->fields[field].type) {
        case DbcFieldType::UInt32:
            return std::to_string(dbc.GetUInt32(record, field));
        case DbcFieldType::Int32:
            return std::to_string(dbc.GetInt32(record, field));
        case DbcFieldType::Float: {
            float val = dbc.GetFloat(record, field);
            if (std::isnan(val) || std::isinf(val)) return "0";
            return std::to_string(val);
        }
        case DbcFieldType::String:
            return "'" + EscapeSql(dbc.GetStringField(record, field)) + "'";
        default:
            return std::to_string(dbc.GetUInt32(record, field));
        }
    }
    return "0";
}

bool DbImportDbc(psql_connector& db, const DbcFile& dbc, const DbcSchema* schema, bool quiet) {
    if (!schema || !db.IsConnected()) return false;

    // Validate schema against DBC
    if (schema->packed) {
        uint32_t expected = GetSchemaRecordSize(schema);
        if (expected != dbc.GetRecordSize()) {
            if (!quiet) {
                fprintf(stderr, "Skipping %s: schema record size %u, DBC record size %u\n",
                        schema->dbc_name, expected, dbc.GetRecordSize());
            }
            return false;
        }
    } else {
        if (schema->field_count != dbc.GetFieldCount()) {
            if (!quiet) {
                fprintf(stderr, "Skipping %s: schema has %u fields, DBC has %u\n",
                        schema->dbc_name, schema->field_count, dbc.GetFieldCount());
            }
            return false;
        }
    }

    std::string table_name = DbcTableName(schema->dbc_name);

    // Build column list once
    std::vector<uint32_t> col_indices;
    std::ostringstream cols;
    bool first = true;
    for (uint32_t f = 0; f < schema->field_count; f++) {
        if (IsPadding(schema->fields[f].name)) continue;
        col_indices.push_back(f);
        if (!first) cols << ", ";
        first = false;
        cols << QuoteIdent(DbcColumnName(schema->fields[f].name));
    }
    std::string col_list = cols.str();

    // Insert all records in a single transaction
    try {
        pqxx::work txn(db.GetConnection());

        // Truncate existing data for a clean import
        txn.exec("DELETE FROM " + QuoteIdent(table_name));

        for (uint32_t r = 0; r < dbc.GetRecordCount(); r++) {
            std::ostringstream sql;
            sql << "INSERT INTO " << QuoteIdent(table_name) << " (" << col_list << ") VALUES (";

            for (size_t i = 0; i < col_indices.size(); i++) {
                if (i > 0) sql << ", ";
                sql << GetFieldValueSql(dbc, schema, r, col_indices[i]);
            }
            sql << ") ON CONFLICT DO NOTHING";

            txn.exec(sql.str());
        }

        txn.commit();

        if (!quiet) {
            printf("  Imported %s: %u records\n", table_name.c_str(), dbc.GetRecordCount());
        }
        return true;
    } catch (const std::exception& e) {
        fprintf(stderr, "Failed to import %s: %s\n", table_name.c_str(), e.what());
        return false;
    }
}
