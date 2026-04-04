#ifndef SCHEMA_ANIMATION_DATA_H
#define SCHEMA_ANIMATION_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef animation_data_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "Name",       DbcFieldType::String },
    { "WeaponFlags", DbcFieldType::UInt32 },
    { "BodyFlags",  DbcFieldType::UInt32 },
    { "Flags",      DbcFieldType::UInt32 },
    { "FallbackID", DbcFieldType::UInt32 },
    { "BehaviorID", DbcFieldType::UInt32 },
    { "BehaviorTier", DbcFieldType::UInt32 },
};

static const DbcSchema schema_animation_data = {
    "AnimationData",
    animation_data_fields,
    sizeof(animation_data_fields) / sizeof(animation_data_fields[0])
};

#endif
