#ifndef SCHEMA_WEAPON_IMPACT_SOUNDS_H
#define SCHEMA_WEAPON_IMPACT_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef weapon_impact_sounds_fields[] = {
    { "Id",                   DbcFieldType::UInt32 },
    { "WeaponSubClassID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "weaponsubclass" },
    { "ParrySoundType",       DbcFieldType::UInt32 },
    { "ImpactSoundID1",       DbcFieldType::UInt32 },
    { "ImpactSoundID2",       DbcFieldType::UInt32 },
    { "ImpactSoundID3",       DbcFieldType::UInt32 },
    { "ImpactSoundID4",       DbcFieldType::UInt32 },
    { "ImpactSoundID5",       DbcFieldType::UInt32 },
    { "ImpactSoundID6",       DbcFieldType::UInt32 },
    { "ImpactSoundID7",       DbcFieldType::UInt32 },
    { "ImpactSoundID8",       DbcFieldType::UInt32 },
    { "ImpactSoundID9",       DbcFieldType::UInt32 },
    { "ImpactSoundID10",      DbcFieldType::UInt32 },
    { "CritImpactSoundID1",   DbcFieldType::UInt32 },
    { "CritImpactSoundID2",   DbcFieldType::UInt32 },
    { "CritImpactSoundID3",   DbcFieldType::UInt32 },
    { "CritImpactSoundID4",   DbcFieldType::UInt32 },
    { "CritImpactSoundID5",   DbcFieldType::UInt32 },
    { "CritImpactSoundID6",   DbcFieldType::UInt32 },
    { "CritImpactSoundID7",   DbcFieldType::UInt32 },
    { "CritImpactSoundID8",   DbcFieldType::UInt32 },
    { "CritImpactSoundID9",   DbcFieldType::UInt32 },
    { "CritImpactSoundID10",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_weapon_impact_sounds = {
    "WeaponImpactSounds",
    weapon_impact_sounds_fields,
    sizeof(weapon_impact_sounds_fields) / sizeof(weapon_impact_sounds_fields[0])
};

#endif
