#ifndef SCHEMA_CHAR_HAIR_GEOSETS_H
#define SCHEMA_CHAR_HAIR_GEOSETS_H

#include "dbc_schema.h"

static const DbcFieldDef char_hair_geosets_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "RaceID",    DbcFieldType::UInt32 },
    { "SexID",     DbcFieldType::UInt32 },
    { "VariationID", DbcFieldType::UInt32 },
    { "GeosetID",  DbcFieldType::UInt32 },
    { "Bald",      DbcFieldType::UInt32 },
};

static const DbcSchema schema_char_hair_geosets = {
    "CharHairGeosets",
    char_hair_geosets_fields,
    sizeof(char_hair_geosets_fields) / sizeof(char_hair_geosets_fields[0])
};

#endif
