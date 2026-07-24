#ifndef SCHEMA_MATERIAL_H
#define SCHEMA_MATERIAL_H

#include "dbc_schema.h"

static const DbcFieldDef material_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "FoleySoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "foleysound", "Visuals" },
    { "SheatheSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "sheathesound", "Visuals" },
    { "UnsheatheSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "unsheathesound", "Visuals" },
};

static const DbcSchema schema_material = {
    "Material",
    material_fields,
    sizeof(material_fields) / sizeof(material_fields[0])
};

#endif
