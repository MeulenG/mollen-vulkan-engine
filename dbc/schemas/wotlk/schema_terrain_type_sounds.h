#ifndef SCHEMA_TERRAIN_TYPE_SOUNDS_H
#define SCHEMA_TERRAIN_TYPE_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef terrain_type_sounds_fields[] = {
    { "Id",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_terrain_type_sounds = {
    "TerrainTypeSounds",
    terrain_type_sounds_fields,
    sizeof(terrain_type_sounds_fields) / sizeof(terrain_type_sounds_fields[0])
};

#endif
