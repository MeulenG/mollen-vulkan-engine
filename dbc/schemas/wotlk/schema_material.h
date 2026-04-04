#ifndef SCHEMA_MATERIAL_H
#define SCHEMA_MATERIAL_H

#include "dbc_schema.h"

static const DbcFieldDef material_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Flags",           DbcFieldType::UInt32 },
    { "FoleySoundID",    DbcFieldType::UInt32 },
    { "SheatheSoundID",  DbcFieldType::UInt32 },
    { "UnsheatheSoundID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_material = {
    "Material",
    material_fields,
    sizeof(material_fields) / sizeof(material_fields[0])
};

#endif
