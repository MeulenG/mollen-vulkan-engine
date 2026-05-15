#ifndef SCHEMA_WEATHER_H
#define SCHEMA_WEATHER_H

#include "dbc_schema.h"

static const DbcFieldDef weather_fields[] = {
    { "Id",                DbcFieldType::UInt32 },
    { "AmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "ambience" },
    { "EffectType",        DbcFieldType::UInt32 },
    { "TransitionSkyBox",  DbcFieldType::Float  },
    { "EffectColor",       DbcFieldType::Float  },
    { "EffectColorG",      DbcFieldType::Float  },
    { "EffectColorB",      DbcFieldType::Float  },
    { "EffectTexture",     DbcFieldType::String },
};

static const DbcSchema schema_weather = {
    "Weather",
    weather_fields,
    sizeof(weather_fields) / sizeof(weather_fields[0])
};

#endif
