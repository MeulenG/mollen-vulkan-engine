#ifndef SCHEMA_OBJECT_EFFECT_GROUP_H
#define SCHEMA_OBJECT_EFFECT_GROUP_H

#include "dbc_schema.h"

static const DbcFieldDef object_effect_group_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
};

static const DbcSchema schema_object_effect_group = {
    "ObjectEffectGroup",
    object_effect_group_fields,
    sizeof(object_effect_group_fields) / sizeof(object_effect_group_fields[0])
};

#endif
