#ifndef SCHEMA_GROUND_EFFECT_TEXTURE_H
#define SCHEMA_GROUND_EFFECT_TEXTURE_H

#include "dbc_schema.h"

static const DbcFieldDef ground_effect_texture_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "DoodadID1",     DbcFieldType::UInt32 },
    { "DoodadID2",     DbcFieldType::UInt32 },
    { "DoodadID3",     DbcFieldType::UInt32 },
    { "DoodadID4",     DbcFieldType::UInt32 },
    { "DoodadWeight1", DbcFieldType::UInt32 },
    { "DoodadWeight2", DbcFieldType::UInt32 },
    { "DoodadWeight3", DbcFieldType::UInt32 },
    { "DoodadWeight4", DbcFieldType::UInt32 },
    { "Density",       DbcFieldType::UInt32 },
    { "Sound",         DbcFieldType::UInt32 },
};

static const DbcSchema schema_ground_effect_texture = {
    "GroundEffectTexture",
    ground_effect_texture_fields,
    sizeof(ground_effect_texture_fields) / sizeof(ground_effect_texture_fields[0])
};

#endif
