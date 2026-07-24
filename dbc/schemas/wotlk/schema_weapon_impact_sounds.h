#ifndef SCHEMA_WEAPON_IMPACT_SOUNDS_H
#define SCHEMA_WEAPON_IMPACT_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef weapon_impact_sounds_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "WeaponSubClassID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "weaponsubclass" },
    { "ParrySoundType", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID5", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID6", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID7", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID8", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID9", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ImpactSoundID10", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID5", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID6", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID7", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID8", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID9", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CritImpactSoundID10", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_weapon_impact_sounds = {
    "WeaponImpactSounds",
    weapon_impact_sounds_fields,
    sizeof(weapon_impact_sounds_fields) / sizeof(weapon_impact_sounds_fields[0])
};

#endif
