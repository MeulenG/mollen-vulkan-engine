#ifndef SCHEMA_TERRAIN_TYPE_H
#define SCHEMA_TERRAIN_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef terrain_type_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "TerrainID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "terrain" },
    { "TerrainDesc", DbcFieldType::String },
    { "FootstepSpray", DbcFieldType::UInt32 },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_terrain_type = {
    "TerrainType",
    terrain_type_fields,
    sizeof(terrain_type_fields) / sizeof(terrain_type_fields[0])
};

#endif
