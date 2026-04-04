#ifndef SCHEMA_SPELL_CAST_TIMES_H
#define SCHEMA_SPELL_CAST_TIMES_H

#include "dbc_schema.h"

static const DbcFieldDef spell_cast_times_fields[] = {
    { "Id",           DbcFieldType::UInt32 },
    { "CastTime",     DbcFieldType::Int32  },
    { "CastTimePerLevel", DbcFieldType::Float },
    { "MinCastTime",  DbcFieldType::Int32  },
};

static const DbcSchema schema_spell_cast_times = {
    "SpellCastTimes",
    spell_cast_times_fields,
    sizeof(spell_cast_times_fields) / sizeof(spell_cast_times_fields[0])
};

#endif
