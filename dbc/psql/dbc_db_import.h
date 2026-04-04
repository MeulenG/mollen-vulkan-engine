#ifndef DBC_DB_IMPORT_H
#define DBC_DB_IMPORT_H

#include "dbc_file.h"
#include "dbc_schema.h"
#include "psql_connector.h"

bool DbCreateTable(psql_connector& db, const DbcSchema* schema);
bool DbImportDbc(psql_connector& db, const DbcFile& dbc, const DbcSchema* schema, bool quiet = true);

#endif // DBC_DB_IMPORT_H
