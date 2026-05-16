#ifndef SCHEMA_DESTRUCTIBLE_MODEL_DATA_H
#define SCHEMA_DESTRUCTIBLE_MODEL_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef destructible_model_data_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "State0ImpactEffectDoodadSet", DbcFieldType::UInt32 },
    { "State0AmbientDoodadSet", DbcFieldType::UInt32 },
    { "State1Wmo", DbcFieldType::UInt32 },
    { "State1DestructionDoodadSet", DbcFieldType::UInt32 },
    { "State1ImpactEffectDoodadSet", DbcFieldType::UInt32 },
    { "State1AmbientDoodadSet", DbcFieldType::UInt32 },
    { "State2Wmo", DbcFieldType::UInt32 },
    { "State2DestructionDoodadSet", DbcFieldType::UInt32 },
    { "State2ImpactEffectDoodadSet", DbcFieldType::UInt32 },
    { "State2AmbientDoodadSet", DbcFieldType::UInt32 },
    { "State3Wmo", DbcFieldType::UInt32 },
    { "State3DestructionDoodadSet", DbcFieldType::UInt32 },
    { "State3ImpactEffectDoodadSet", DbcFieldType::UInt32 },
    { "State3AmbientDoodadSet", DbcFieldType::UInt32 },
    { "EjectDirection", DbcFieldType::UInt32 },
    { "RepairGroundFx", DbcFieldType::UInt32 },
    { "DoNotHighlight", DbcFieldType::UInt32 },
    { "HealEffect", DbcFieldType::UInt32 },
};

static const DbcSchema schema_destructible_model_data = {
    "DestructibleModelData",
    destructible_model_data_fields,
    sizeof(destructible_model_data_fields) / sizeof(destructible_model_data_fields[0])
};

#endif
