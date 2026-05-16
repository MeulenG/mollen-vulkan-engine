#ifndef SCHEMA_STATIONERY_H
#define SCHEMA_STATIONERY_H

#include "dbc_schema.h"

static const DbcFieldDef stationery_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ItemID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "item" },
    { "Texture", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_stationery = {
    "Stationery",
    stationery_fields,
    sizeof(stationery_fields) / sizeof(stationery_fields[0])
};

#endif
