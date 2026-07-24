#ifndef SCHEMA_GROUND_EFFECT_DOODAD_H
#define SCHEMA_GROUND_EFFECT_DOODAD_H

#include "dbc_schema.h"

static const DbcFieldDef ground_effect_doodad_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Doodadpath", DbcFieldType::String },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_ground_effect_doodad = {
    "GroundEffectDoodad",
    ground_effect_doodad_fields,
    sizeof(ground_effect_doodad_fields) / sizeof(ground_effect_doodad_fields[0])
};

#endif
