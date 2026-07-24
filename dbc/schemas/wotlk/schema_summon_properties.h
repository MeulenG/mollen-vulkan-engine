#ifndef SCHEMA_SUMMON_PROPERTIES_H
#define SCHEMA_SUMMON_PROPERTIES_H

#include "dbc_schema.h"

static const DbcFieldDef summon_properties_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Control", DbcFieldType::UInt32 },
    { "Faction", DbcFieldType::UInt32 },
    { "Title", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Slot", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_summon_properties = {
    "SummonProperties",
    summon_properties_fields,
    sizeof(summon_properties_fields) / sizeof(summon_properties_fields[0])
};

#endif
