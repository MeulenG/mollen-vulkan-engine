#ifndef SCHEMA_OBJECT_EFFECT_PACKAGE_ELEM_H
#define SCHEMA_OBJECT_EFFECT_PACKAGE_ELEM_H

#include "dbc_schema.h"

static const DbcFieldDef object_effect_package_elem_fields[] = {
    { "Id",                    DbcFieldType::UInt32 },
    { "ObjectEffectPackageID", DbcFieldType::UInt32 },
    { "ObjectEffectGroupID",   DbcFieldType::UInt32 },
    { "StateType",             DbcFieldType::UInt32 },
};

static const DbcSchema schema_object_effect_package_elem = {
    "ObjectEffectPackageElem",
    object_effect_package_elem_fields,
    sizeof(object_effect_package_elem_fields) / sizeof(object_effect_package_elem_fields[0])
};

#endif
