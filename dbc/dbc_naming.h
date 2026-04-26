#ifndef DBC_NAMING_H
#define DBC_NAMING_H

#include <string>

// Mapping from DBC schema identifiers (PascalCase) to PostgreSQL identifiers
// used by dbc_db_import. Centralized so the editor and importer agree.
//
//   "CreatureDisplayInfo"  -> table  "creaturedisplayinfo"
//   "CreatureModelScale"   -> column "creature_model_scale"
//
// Any consumer that looks up a row by table name or builds an UPDATE statement
// MUST go through these helpers — copy-pasted casing has caused real bugs.

std::string DbcTableName(const char* schema_name);
std::string DbcColumnName(const char* field_name);

#endif // DBC_NAMING_H
