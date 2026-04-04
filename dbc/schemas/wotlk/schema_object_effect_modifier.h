#ifndef SCHEMA_OBJECT_EFFECT_MODIFIER_H
#define SCHEMA_OBJECT_EFFECT_MODIFIER_H

#include "dbc_schema.h"

static const DbcFieldDef object_effect_modifier_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "InputType",      DbcFieldType::UInt32 },
    { "MapType",        DbcFieldType::UInt32 },
    { "OutputType",     DbcFieldType::UInt32 },
    { "Param1",         DbcFieldType::Float  },
    { "Param2",         DbcFieldType::Float  },
    { "Param3",         DbcFieldType::Float  },
    { "Param4",         DbcFieldType::Float  },
};

static const DbcSchema schema_object_effect_modifier = {
    "ObjectEffectModifier",
    object_effect_modifier_fields,
    sizeof(object_effect_modifier_fields) / sizeof(object_effect_modifier_fields[0])
};

#endif
