#ifndef SCHEMA_WEAPON_SWING_SOUNDS2_H
#define SCHEMA_WEAPON_SWING_SOUNDS2_H

#include "dbc_schema.h"

static const DbcFieldDef weapon_swing_sounds2_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SwingType", DbcFieldType::UInt32 },
    { "Crit", DbcFieldType::UInt32 },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
};

static const DbcSchema schema_weapon_swing_sounds2 = {
    "WeaponSwingSounds2",
    weapon_swing_sounds2_fields,
    sizeof(weapon_swing_sounds2_fields) / sizeof(weapon_swing_sounds2_fields[0])
};

#endif
