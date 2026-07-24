#ifndef SCHEMA_CHAR_VARIATIONS_H
#define SCHEMA_CHAR_VARIATIONS_H

#include "dbc_schema.h"

static const DbcFieldDef char_variations_fields[] = {
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "SexID", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
    { "TextureHoldID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "texturehold" },
    { "Unknown1",  DbcFieldType::UInt32 },
    { "Unknown2",  DbcFieldType::UInt32 },
    { "Unknown3",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_char_variations = {
    "CharVariations",
    char_variations_fields,
    sizeof(char_variations_fields) / sizeof(char_variations_fields[0])
};

#endif
