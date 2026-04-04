#ifndef SCHEMA_OBJECT_EFFECT_PACKAGE_H
#define SCHEMA_OBJECT_EFFECT_PACKAGE_H

#include "dbc_schema.h"

static const DbcFieldDef object_effect_package_fields[] = {
    { "Id",    DbcFieldType::UInt32 },
    { "Name",  DbcFieldType::String },
};

static const DbcSchema schema_object_effect_package = {
    "ObjectEffectPackage",
    object_effect_package_fields,
    sizeof(object_effect_package_fields) / sizeof(object_effect_package_fields[0])
};

#endif
