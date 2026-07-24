#ifndef SCHEMA_DURABILITY_QUALITY_H
#define SCHEMA_DURABILITY_QUALITY_H

#include "dbc_schema.h"

static const DbcFieldDef durability_quality_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Data", DbcFieldType::Float },
};

static const DbcSchema schema_durability_quality = {
    "DurabilityQuality",
    durability_quality_fields,
    sizeof(durability_quality_fields) / sizeof(durability_quality_fields[0])
};

#endif
