#ifndef SCHEMA_CHARACTER_FACIAL_HAIR_STYLES_H
#define SCHEMA_CHARACTER_FACIAL_HAIR_STYLES_H

#include "dbc_schema.h"

static const DbcFieldDef character_facial_hair_styles_fields[] = {
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "SexID", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
    { "VariationID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "variation" },
    { "Geoset1",   DbcFieldType::UInt32 },
    { "Geoset2",   DbcFieldType::UInt32 },
    { "Geoset3",   DbcFieldType::UInt32 },
    { "Geoset4",   DbcFieldType::UInt32 },
    { "Geoset5",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_character_facial_hair_styles = {
    "CharacterFacialHairStyles",
    character_facial_hair_styles_fields,
    sizeof(character_facial_hair_styles_fields) / sizeof(character_facial_hair_styles_fields[0])
};

#endif
