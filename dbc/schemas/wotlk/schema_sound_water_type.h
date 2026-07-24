#ifndef SCHEMA_SOUND_WATER_TYPE_H
#define SCHEMA_SOUND_WATER_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef sound_water_type_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SoundType", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundSubType", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundEntriesID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
};

static const DbcSchema schema_sound_water_type = {
    "SoundWaterType",
    sound_water_type_fields,
    sizeof(sound_water_type_fields) / sizeof(sound_water_type_fields[0])
};

#endif
