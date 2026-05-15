#ifndef SCHEMA_LIQUID_MATERIAL_H
#define SCHEMA_LIQUID_MATERIAL_H

#include "dbc_schema.h"

static const DbcFieldDef liquid_material_fields[] = {
    { "Id",       DbcFieldType::UInt32 },
    { "LVF",      DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_liquid_material = {
    "LiquidMaterial",
    liquid_material_fields,
    sizeof(liquid_material_fields) / sizeof(liquid_material_fields[0])
};

#endif
