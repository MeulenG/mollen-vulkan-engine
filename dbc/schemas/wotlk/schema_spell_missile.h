#ifndef SCHEMA_SPELL_MISSILE_H
#define SCHEMA_SPELL_MISSILE_H

#include "dbc_schema.h"

static const DbcFieldDef spell_missile_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "DefaultPitchMin", DbcFieldType::Float },
    { "DefaultPitchMax", DbcFieldType::Float },
    { "DefaultSpeedMin", DbcFieldType::Float },
    { "DefaultSpeedMax", DbcFieldType::Float },
    { "RandomizeFacingMin", DbcFieldType::Float },
    { "RandomizeFacingMax", DbcFieldType::Float },
    { "RandomizePitchMin", DbcFieldType::Float },
    { "RandomizePitchMax", DbcFieldType::Float },
    { "RandomizeSpeedMin", DbcFieldType::Float },
    { "RandomizeSpeedMax", DbcFieldType::Float },
    { "Gravity", DbcFieldType::Float },
    { "MaxDuration", DbcFieldType::Float },
    { "CollisionRadius", DbcFieldType::Float },
};

static const DbcSchema schema_spell_missile = {
    "SpellMissile",
    spell_missile_fields,
    sizeof(spell_missile_fields) / sizeof(spell_missile_fields[0])
};

#endif
