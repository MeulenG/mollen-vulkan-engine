#ifndef SCHEMA_DECLINED_WORD_CASES_H
#define SCHEMA_DECLINED_WORD_CASES_H

#include "dbc_schema.h"

static const DbcFieldDef declined_word_cases_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "DeclinedWordID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "declinedword" },
    { "CaseIndex", DbcFieldType::UInt32 },
    { "DeclinedWord", DbcFieldType::String },
};

static const DbcSchema schema_declined_word_cases = {
    "DeclinedWordCases",
    declined_word_cases_fields,
    sizeof(declined_word_cases_fields) / sizeof(declined_word_cases_fields[0])
};

#endif
