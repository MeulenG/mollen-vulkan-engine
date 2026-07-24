#ifndef SCHEMA_EMOTES_TEXT_H
#define SCHEMA_EMOTES_TEXT_H

#include "dbc_schema.h"

static const DbcFieldDef emotes_text_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "EmoteID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "emote" },
    { "EmoteText1", DbcFieldType::UInt32 },
    { "EmoteText2", DbcFieldType::UInt32 },
    { "EmoteText3", DbcFieldType::UInt32 },
    { "EmoteText4", DbcFieldType::UInt32 },
    { "EmoteText5", DbcFieldType::UInt32 },
    { "EmoteText6", DbcFieldType::UInt32 },
    { "EmoteText7", DbcFieldType::UInt32 },
    { "EmoteText8", DbcFieldType::UInt32 },
    { "EmoteText9", DbcFieldType::UInt32 },
    { "EmoteText10", DbcFieldType::UInt32 },
    { "EmoteText11", DbcFieldType::UInt32 },
    { "EmoteText12", DbcFieldType::UInt32 },
    { "EmoteText13", DbcFieldType::UInt32 },
    { "EmoteText14", DbcFieldType::UInt32 },
    { "EmoteText15", DbcFieldType::UInt32 },
    { "EmoteText16", DbcFieldType::UInt32 },
};

static const DbcSchema schema_emotes_text = {
    "EmotesText",
    emotes_text_fields,
    sizeof(emotes_text_fields) / sizeof(emotes_text_fields[0])
};

#endif
