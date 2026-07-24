#ifndef SCHEMA_SCREEN_EFFECT_H
#define SCHEMA_SCREEN_EFFECT_H

#include "dbc_schema.h"

static const DbcFieldDef screen_effect_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Effect", DbcFieldType::UInt32 },
    { "Param1", DbcFieldType::UInt32 },
    { "Param2", DbcFieldType::UInt32 },
    { "Param3", DbcFieldType::UInt32 },
    { "Param4", DbcFieldType::UInt32 },
    { "LightParamsID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "lightparams" },
    { "SoundAmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundambience", "Visuals" },
    { "ZoneMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zonemusic", "Visuals" },
};

static const DbcSchema schema_screen_effect = {
    "ScreenEffect",
    screen_effect_fields,
    sizeof(screen_effect_fields) / sizeof(screen_effect_fields[0])
};

#endif
