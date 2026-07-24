#ifndef SCHEMA_DEATH_THUD_LOOKUPS_H
#define SCHEMA_DEATH_THUD_LOOKUPS_H

#include "dbc_schema.h"

static const DbcFieldDef death_thud_lookups_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SizeClass", DbcFieldType::UInt32 },
    { "TerrainTypeSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "terraintypesound", "Visuals" },
    { "SoundEntryID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentry", "Visuals" },
    { "SoundEntryIDWater", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_death_thud_lookups = {
    "DeathThudLookups",
    death_thud_lookups_fields,
    sizeof(death_thud_lookups_fields) / sizeof(death_thud_lookups_fields[0])
};

#endif
