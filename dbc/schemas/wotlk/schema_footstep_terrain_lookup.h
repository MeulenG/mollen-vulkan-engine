#ifndef SCHEMA_FOOTSTEP_TERRAIN_LOOKUP_H
#define SCHEMA_FOOTSTEP_TERRAIN_LOOKUP_H

#include "dbc_schema.h"

static const DbcFieldDef footstep_terrain_lookup_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "CreatureFootstepID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "creaturefootstep" },
    { "TerrainSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "terrainsound", "Visuals" },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
    { "SoundIDSplash", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_footstep_terrain_lookup = {
    "FootstepTerrainLookup",
    footstep_terrain_lookup_fields,
    sizeof(footstep_terrain_lookup_fields) / sizeof(footstep_terrain_lookup_fields[0])
};

#endif
