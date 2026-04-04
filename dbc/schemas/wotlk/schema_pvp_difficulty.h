#ifndef SCHEMA_PVP_DIFFICULTY_H
#define SCHEMA_PVP_DIFFICULTY_H

#include "dbc_schema.h"

static const DbcFieldDef pvp_difficulty_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "MapID",          DbcFieldType::UInt32 },
    { "RangeIndex",     DbcFieldType::UInt32 },
    { "MinLevel",       DbcFieldType::UInt32 },
    { "MaxLevel",       DbcFieldType::UInt32 },
    { "Difficulty",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_pvp_difficulty = {
    "PvpDifficulty",
    pvp_difficulty_fields,
    sizeof(pvp_difficulty_fields) / sizeof(pvp_difficulty_fields[0])
};

#endif
