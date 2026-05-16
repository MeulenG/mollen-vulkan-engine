#ifndef SCHEMA_EMOTES_TEXT_SOUND_H
#define SCHEMA_EMOTES_TEXT_SOUND_H

#include "dbc_schema.h"

static const DbcFieldDef emotes_text_sound_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "EmotesTextID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "emotestext" },
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "SexID", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
};

static const DbcSchema schema_emotes_text_sound = {
    "EmotesTextSound",
    emotes_text_sound_fields,
    sizeof(emotes_text_sound_fields) / sizeof(emotes_text_sound_fields[0])
};

#endif
