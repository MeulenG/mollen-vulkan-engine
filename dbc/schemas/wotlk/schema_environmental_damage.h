#ifndef SCHEMA_ENVIRONMENTAL_DAMAGE_H
#define SCHEMA_ENVIRONMENTAL_DAMAGE_H

#include "dbc_schema.h"

static const DbcFieldDef environmental_damage_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "EnumID",          DbcFieldType::UInt32 },
    { "VisualkitID",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_environmental_damage = {
    "EnvironmentalDamage",
    environmental_damage_fields,
    sizeof(environmental_damage_fields) / sizeof(environmental_damage_fields[0])
};

#endif
