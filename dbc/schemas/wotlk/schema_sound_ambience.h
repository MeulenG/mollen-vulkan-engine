#ifndef SCHEMA_SOUND_AMBIENCE_H
#define SCHEMA_SOUND_AMBIENCE_H

#include "dbc_schema.h"

static const DbcFieldDef sound_ambience_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "DaySoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "daysound", "Visuals" },
    { "NightSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "nightsound", "Visuals" },
};

static const DbcSchema schema_sound_ambience = {
    "SoundAmbience",
    sound_ambience_fields,
    sizeof(sound_ambience_fields) / sizeof(sound_ambience_fields[0])
};

#endif
