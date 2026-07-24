#ifndef SCHEMA_DECLINED_WORD_H
#define SCHEMA_DECLINED_WORD_H

#include "dbc_schema.h"

static const DbcFieldDef declined_word_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Word", DbcFieldType::String },
};

static const DbcSchema schema_declined_word = {
    "DeclinedWord",
    declined_word_fields,
    sizeof(declined_word_fields) / sizeof(declined_word_fields[0])
};

#endif
