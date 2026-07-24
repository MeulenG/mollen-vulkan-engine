#ifndef SCHEMA_SPELL_MISSILE_MOTION_H
#define SCHEMA_SPELL_MISSILE_MOTION_H

#include "dbc_schema.h"

static const DbcFieldDef spell_missile_motion_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Script", DbcFieldType::String },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "MissileCount", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_missile_motion = {
    "SpellMissileMotion",
    spell_missile_motion_fields,
    sizeof(spell_missile_motion_fields) / sizeof(spell_missile_motion_fields[0])
};

#endif
