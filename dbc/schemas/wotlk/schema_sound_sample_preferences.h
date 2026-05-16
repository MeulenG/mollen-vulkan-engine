#ifndef SCHEMA_SOUND_SAMPLE_PREFERENCES_H
#define SCHEMA_SOUND_SAMPLE_PREFERENCES_H

#include "dbc_schema.h"

static const DbcFieldDef sound_sample_preferences_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Field01", DbcFieldType::UInt32 },
    { "Field02", DbcFieldType::UInt32 },
    { "Field03", DbcFieldType::UInt32 },
    { "Field04", DbcFieldType::UInt32 },
    { "Field05", DbcFieldType::UInt32 },
    { "EAXDef", DbcFieldType::UInt32 },
    { "Field07", DbcFieldType::Float },
    { "Field08", DbcFieldType::Float },
    { "Field09", DbcFieldType::Float },
    { "Field10", DbcFieldType::Float },
    { "Field11", DbcFieldType::UInt32 },
    { "Field12", DbcFieldType::UInt32 },
    { "Field13", DbcFieldType::Float },
    { "Field14", DbcFieldType::UInt32 },
    { "Field15", DbcFieldType::Float },
    { "Field16", DbcFieldType::Float },
};

static const DbcSchema schema_sound_sample_preferences = {
    "SoundSamplePreferences",
    sound_sample_preferences_fields,
    sizeof(sound_sample_preferences_fields) / sizeof(sound_sample_preferences_fields[0])
};

#endif
