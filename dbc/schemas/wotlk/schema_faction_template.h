#ifndef SCHEMA_FACTION_TEMPLATE_H
#define SCHEMA_FACTION_TEMPLATE_H

#include "dbc_schema.h"

static const DbcFieldDef faction_template_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Faction",       DbcFieldType::UInt32 },
    { "FactionGroup",  DbcFieldType::UInt32 },
    { "FriendGroup",   DbcFieldType::UInt32 },
    { "EnemyGroup",    DbcFieldType::UInt32 },
    { "Enemies1",      DbcFieldType::UInt32 },
    { "Enemies2",      DbcFieldType::UInt32 },
    { "Enemies3",      DbcFieldType::UInt32 },
    { "Enemies4",      DbcFieldType::UInt32 },
    { "Friends1",      DbcFieldType::UInt32 },
    { "Friends2",      DbcFieldType::UInt32 },
    { "Friends3",      DbcFieldType::UInt32 },
    { "Friends4",      DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, "FactionTemplateFlags" },
};

static const DbcSchema schema_faction_template = {
    "FactionTemplate",
    faction_template_fields,
    sizeof(faction_template_fields) / sizeof(faction_template_fields[0])
};

#endif
