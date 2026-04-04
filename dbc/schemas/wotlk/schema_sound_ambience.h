#ifndef SCHEMA_SOUND_AMBIENCE_H
#define SCHEMA_SOUND_AMBIENCE_H

#include "dbc_schema.h"

static const DbcFieldDef sound_ambience_fields[] = {
    { "Id",           DbcFieldType::UInt32 },
    { "DaySoundID",   DbcFieldType::UInt32 },
    { "NightSoundID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_sound_ambience = {
    "SoundAmbience",
    sound_ambience_fields,
    sizeof(sound_ambience_fields) / sizeof(sound_ambience_fields[0])
};

#endif
