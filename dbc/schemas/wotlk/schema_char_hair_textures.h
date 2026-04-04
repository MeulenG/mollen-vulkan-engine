#ifndef SCHEMA_CHAR_HAIR_TEXTURES_H
#define SCHEMA_CHAR_HAIR_TEXTURES_H

#include "dbc_schema.h"

static const DbcFieldDef char_hair_textures_fields[] = {
    { "Id",           DbcFieldType::UInt32 },
    { "RaceID",       DbcFieldType::UInt32 },
    { "SexID",        DbcFieldType::UInt32 },
    { "VariationID",  DbcFieldType::UInt32 },
    { "GeosetGroup1", DbcFieldType::UInt32 },
    { "GeosetGroup2", DbcFieldType::UInt32 },
    { "GeosetGroup3", DbcFieldType::UInt32 },
    { "Unknown1",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_char_hair_textures = {
    "CharHairTextures",
    char_hair_textures_fields,
    sizeof(char_hair_textures_fields) / sizeof(char_hair_textures_fields[0])
};

#endif
