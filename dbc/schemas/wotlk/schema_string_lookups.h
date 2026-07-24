#ifndef SCHEMA_STRING_LOOKUPS_H
#define SCHEMA_STRING_LOOKUPS_H

#include "dbc_schema.h"

static const DbcFieldDef string_lookups_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "String", DbcFieldType::String },
};

static const DbcSchema schema_string_lookups = {
    "StringLookups",
    string_lookups_fields,
    sizeof(string_lookups_fields) / sizeof(string_lookups_fields[0])
};

#endif
