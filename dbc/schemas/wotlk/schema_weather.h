#ifndef SCHEMA_WEATHER_H
#define SCHEMA_WEATHER_H

#include "dbc_schema.h"

static const DbcFieldDef weather_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "AmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "ambience" },
    { "EffectType", DbcFieldType::UInt32 },
    { "TransitionSkyBox", DbcFieldType::Float },
    { "EffectColor", DbcFieldType::Float, DbcSemantic::Default, nullptr, "Visuals" },
    { "EffectColorG", DbcFieldType::Float, DbcSemantic::Default, nullptr, "Visuals" },
    { "EffectColorB", DbcFieldType::Float, DbcSemantic::Default, nullptr, "Visuals" },
    { "EffectTexture", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_weather = {
    "Weather",
    weather_fields,
    sizeof(weather_fields) / sizeof(weather_fields[0])
};

#endif
