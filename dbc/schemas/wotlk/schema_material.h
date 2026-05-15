#ifndef SCHEMA_MATERIAL_H
#define SCHEMA_MATERIAL_H

#include "dbc_schema.h"

static const DbcFieldDef material_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "FoleySoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "foleysound" },
    { "SheatheSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "sheathesound" },
    { "UnsheatheSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "unsheathesound" },
};

static const DbcSchema schema_material = {
    "Material",
    material_fields,
    sizeof(material_fields) / sizeof(material_fields[0])
};

#endif
