#ifndef SCHEMA_NAMES_PROFANITY_H
#define SCHEMA_NAMES_PROFANITY_H

#include "dbc_schema.h"

static const DbcFieldDef names_profanity_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Language", DbcFieldType::UInt32 },
};

static const DbcSchema schema_names_profanity = {
    "NamesProfanity",
    names_profanity_fields,
    sizeof(names_profanity_fields) / sizeof(names_profanity_fields[0])
};

#endif
