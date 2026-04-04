#ifndef SCHEMA_OBJECT_EFFECT_H
#define SCHEMA_OBJECT_EFFECT_H

#include "dbc_schema.h"

static const DbcFieldDef object_effect_fields[] = {
    { "Id",                DbcFieldType::UInt32 },
    { "Name",              DbcFieldType::String },
    { "ObjectEffectGroupID", DbcFieldType::UInt32 },
    { "TriggerType",       DbcFieldType::UInt32 },
    { "EventType",         DbcFieldType::UInt32 },
    { "EffectRecType",     DbcFieldType::UInt32 },
    { "EffectRecID",       DbcFieldType::UInt32 },
    { "Attachment",        DbcFieldType::UInt32 },
    { "OffsetX",           DbcFieldType::Float  },
    { "OffsetY",           DbcFieldType::Float  },
    { "OffsetZ",           DbcFieldType::Float  },
    { "ObjectEffectModifierID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_object_effect = {
    "ObjectEffect",
    object_effect_fields,
    sizeof(object_effect_fields) / sizeof(object_effect_fields[0])
};

#endif
