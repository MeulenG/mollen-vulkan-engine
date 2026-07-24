#ifndef SCHEMA_SPAM_MESSAGES_H
#define SCHEMA_SPAM_MESSAGES_H

#include "dbc_schema.h"

static const DbcFieldDef spam_messages_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Text", DbcFieldType::String },
};

static const DbcSchema schema_spam_messages = {
    "SpamMessages",
    spam_messages_fields,
    sizeof(spam_messages_fields) / sizeof(spam_messages_fields[0])
};

#endif
