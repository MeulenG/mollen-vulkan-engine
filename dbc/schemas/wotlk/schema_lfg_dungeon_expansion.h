#ifndef SCHEMA_LFG_DUNGEON_EXPANSION_H
#define SCHEMA_LFG_DUNGEON_EXPANSION_H

#include "dbc_schema.h"

static const DbcFieldDef lfg_dungeon_expansion_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "LFGDungeonID",    DbcFieldType::UInt32 },
    { "ExpansionLevel",  DbcFieldType::UInt32 },
    { "RandomID",        DbcFieldType::UInt32 },
    { "HardLevelMin",    DbcFieldType::UInt32 },
    { "HardLevelMax",    DbcFieldType::UInt32 },
    { "TargetLevelMin",  DbcFieldType::UInt32 },
    { "TargetLevelMax",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_lfg_dungeon_expansion = {
    "LFGDungeonExpansion",
    lfg_dungeon_expansion_fields,
    sizeof(lfg_dungeon_expansion_fields) / sizeof(lfg_dungeon_expansion_fields[0])
};

#endif
