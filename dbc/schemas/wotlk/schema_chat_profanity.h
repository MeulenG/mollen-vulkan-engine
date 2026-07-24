#ifndef SCHEMA_CHAT_PROFANITY_H
#define SCHEMA_CHAT_PROFANITY_H

#include "dbc_schema.h"

static const DbcFieldDef chat_profanity_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Text", DbcFieldType::String },
    { "Language", DbcFieldType::UInt32 },
};

static const DbcSchema schema_chat_profanity = {
    "ChatProfanity",
    chat_profanity_fields,
    sizeof(chat_profanity_fields) / sizeof(chat_profanity_fields[0])
};

#endif
