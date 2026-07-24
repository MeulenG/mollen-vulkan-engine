#ifndef SCHEMA_GEM_PROPERTIES_H
#define SCHEMA_GEM_PROPERTIES_H

#include "dbc_schema.h"

static const DbcFieldDef gem_properties_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "EnchantID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "enchant" },
    { "MaxCountInventory", DbcFieldType::UInt32 },
    { "MaxCountItem", DbcFieldType::UInt32 },
    { "Type", DbcFieldType::UInt32 },
};

static const DbcSchema schema_gem_properties = {
    "GemProperties",
    gem_properties_fields,
    sizeof(gem_properties_fields) / sizeof(gem_properties_fields[0])
};

#endif
