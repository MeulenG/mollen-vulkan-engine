#ifndef SCHEMA_STATIONERY_H
#define SCHEMA_STATIONERY_H

#include "dbc_schema.h"

static const DbcFieldDef stationery_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "ItemID",      DbcFieldType::UInt32 },
    { "Texture",     DbcFieldType::String },
    { "Flags",       DbcFieldType::UInt32 },
};

static const DbcSchema schema_stationery = {
    "Stationery",
    stationery_fields,
    sizeof(stationery_fields) / sizeof(stationery_fields[0])
};

#endif
