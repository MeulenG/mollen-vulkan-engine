#ifndef SCHEMA_LANGUAGE_WORDS_H
#define SCHEMA_LANGUAGE_WORDS_H

#include "dbc_schema.h"

static const DbcFieldDef language_words_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "LanguageID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "languages" },
    { "Word", DbcFieldType::String },
};

static const DbcSchema schema_language_words = {
    "LanguageWords",
    language_words_fields,
    sizeof(language_words_fields) / sizeof(language_words_fields[0])
};

#endif
