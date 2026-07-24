#ifndef SCHEMA_FOOTSTEP_TERRAIN_LOOKUP_H
#define SCHEMA_FOOTSTEP_TERRAIN_LOOKUP_H

#include "dbc_schema.h"

static const DbcFieldDef footstep_terrain_lookup_fields[] = {
    { "Id",                  DbcFieldType::UInt32 },
    { "CreatureFootstepID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "creaturefootstep" },
    { "TerrainSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "terrainsound" },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries" },
    { "SoundIDSplash",       DbcFieldType::UInt32 },
};

static const DbcSchema schema_footstep_terrain_lookup = {
    "FootstepTerrainLookup",
    footstep_terrain_lookup_fields,
    sizeof(footstep_terrain_lookup_fields) / sizeof(footstep_terrain_lookup_fields[0])
};

#endif
