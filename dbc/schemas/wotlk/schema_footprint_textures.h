#ifndef SCHEMA_FOOTPRINT_TEXTURES_H
#define SCHEMA_FOOTPRINT_TEXTURES_H

#include "dbc_schema.h"

static const DbcFieldDef footprint_textures_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "TexturePath", DbcFieldType::String },
};

static const DbcSchema schema_footprint_textures = {
    "FootprintTextures",
    footprint_textures_fields,
    sizeof(footprint_textures_fields) / sizeof(footprint_textures_fields[0])
};

#endif
