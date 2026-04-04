#ifndef SCHEMA_EMOTES_TEXT_SOUND_H
#define SCHEMA_EMOTES_TEXT_SOUND_H

#include "dbc_schema.h"

static const DbcFieldDef emotes_text_sound_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "EmotesTextID", DbcFieldType::UInt32 },
    { "RaceID",     DbcFieldType::UInt32 },
    { "SexID",      DbcFieldType::UInt32 },
    { "SoundID",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_emotes_text_sound = {
    "EmotesTextSound",
    emotes_text_sound_fields,
    sizeof(emotes_text_sound_fields) / sizeof(emotes_text_sound_fields[0])
};

#endif
