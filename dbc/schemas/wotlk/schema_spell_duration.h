#ifndef SCHEMA_SPELL_DURATION_H
#define SCHEMA_SPELL_DURATION_H

#include "dbc_schema.h"

static const DbcFieldDef spell_duration_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Duration", DbcFieldType::Int32 },
    { "DurationPerLevel", DbcFieldType::UInt32 },
    { "MaxDuration", DbcFieldType::Int32 },
};

static const DbcSchema schema_spell_duration = {
    "SpellDuration",
    spell_duration_fields,
    sizeof(spell_duration_fields) / sizeof(spell_duration_fields[0])
};

#endif
