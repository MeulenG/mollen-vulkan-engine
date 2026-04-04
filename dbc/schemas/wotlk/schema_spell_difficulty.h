#ifndef SCHEMA_SPELL_DIFFICULTY_H
#define SCHEMA_SPELL_DIFFICULTY_H

#include "dbc_schema.h"

static const DbcFieldDef spell_difficulty_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "SpellId_Normal10", DbcFieldType::UInt32 },
    { "SpellId_Normal25", DbcFieldType::UInt32 },
    { "SpellId_Heroic10", DbcFieldType::UInt32 },
    { "SpellId_Heroic25", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_difficulty = {
    "SpellDifficulty",
    spell_difficulty_fields,
    sizeof(spell_difficulty_fields) / sizeof(spell_difficulty_fields[0])
};

#endif
