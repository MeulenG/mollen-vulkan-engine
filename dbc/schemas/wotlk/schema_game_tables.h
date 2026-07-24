#ifndef SCHEMA_GAME_TABLES_H
#define SCHEMA_GAME_TABLES_H

#include "dbc_schema.h"

static const DbcFieldDef game_tables_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "NumRows", DbcFieldType::UInt32 },
};

static const DbcSchema schema_game_tables = {
    "GameTables",
    game_tables_fields,
    sizeof(game_tables_fields) / sizeof(game_tables_fields[0])
};

#endif
