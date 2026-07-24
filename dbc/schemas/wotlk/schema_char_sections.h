#ifndef SCHEMA_CHAR_SECTIONS_H
#define SCHEMA_CHAR_SECTIONS_H

#include "dbc_schema.h"

static const DbcFieldDef char_sections_fields[] = {
    { "Id",           DbcFieldType::UInt32 },
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "SexID", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
    { "BaseSection",  DbcFieldType::UInt32 },
    { "TextureName1", DbcFieldType::String },
    { "TextureName2", DbcFieldType::String },
    { "TextureName3", DbcFieldType::String },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "VariationIndex", DbcFieldType::UInt32 },
    { "ColorIndex",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_char_sections = {
    "CharSections",
    char_sections_fields,
    sizeof(char_sections_fields) / sizeof(char_sections_fields[0])
};

#endif
