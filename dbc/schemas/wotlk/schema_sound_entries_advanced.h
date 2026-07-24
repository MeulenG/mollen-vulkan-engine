#ifndef SCHEMA_SOUND_ENTRIES_ADVANCED_H
#define SCHEMA_SOUND_ENTRIES_ADVANCED_H

#include "dbc_schema.h"

static const DbcFieldDef sound_entries_advanced_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SoundEntryID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentry", "Visuals" },
    { "InnerRadius2D", DbcFieldType::Float },
    { "TimeA", DbcFieldType::UInt32 },
    { "TimeB", DbcFieldType::UInt32 },
    { "TimeC", DbcFieldType::UInt32 },
    { "TimeD", DbcFieldType::UInt32 },
    { "RandomOffsetRange", DbcFieldType::UInt32 },
    { "Usage", DbcFieldType::UInt32 },
    { "TimeIntervalMin", DbcFieldType::UInt32 },
    { "TimeIntervalMax", DbcFieldType::UInt32 },
    { "VolumeSliderCategory", DbcFieldType::UInt32 },
    { "DuckToSFX", DbcFieldType::Float },
    { "DuckToMusic", DbcFieldType::Float, DbcSemantic::Default, nullptr, "Visuals" },
    { "DuckToAmbience", DbcFieldType::Float },
    { "InnerRadiusOfInfluence", DbcFieldType::Float },
    { "OuterRadiusOfInfluence", DbcFieldType::Float },
    { "TimeToDuck", DbcFieldType::UInt32 },
    { "TimeToUnduck", DbcFieldType::UInt32 },
    { "InsideAngle", DbcFieldType::Float },
    { "OutsideAngle", DbcFieldType::Float },
    { "OutsideVolume", DbcFieldType::Float },
    { "OuterRadius2D", DbcFieldType::Float },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
};

static const DbcSchema schema_sound_entries_advanced = {
    "SoundEntriesAdvanced",
    sound_entries_advanced_fields,
    sizeof(sound_entries_advanced_fields) / sizeof(sound_entries_advanced_fields[0])
};

#endif
