#ifndef SCHEMA_SPELL_EFFECT_CAMERA_SHAKES_H
#define SCHEMA_SPELL_EFFECT_CAMERA_SHAKES_H

#include "dbc_schema.h"

static const DbcFieldDef spell_effect_camera_shakes_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "CameraShake1",   DbcFieldType::UInt32 },
    { "CameraShake2",   DbcFieldType::UInt32 },
    { "CameraShake3",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_effect_camera_shakes = {
    "SpellEffectCameraShakes",
    spell_effect_camera_shakes_fields,
    sizeof(spell_effect_camera_shakes_fields) / sizeof(spell_effect_camera_shakes_fields[0])
};

#endif
