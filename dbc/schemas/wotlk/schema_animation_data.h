#ifndef SCHEMA_ANIMATION_DATA_H
#define SCHEMA_ANIMATION_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef animation_data_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "WeaponFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "BodyFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "FallbackID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "fallback" },
    { "BehaviorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "behavior" },
    { "BehaviorTier", DbcFieldType::UInt32 },
};

static const DbcSchema schema_animation_data = {
    "AnimationData",
    animation_data_fields,
    sizeof(animation_data_fields) / sizeof(animation_data_fields[0])
};

#endif
