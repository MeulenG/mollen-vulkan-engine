#ifndef SCHEMA_NAMES_RESERVED_H
#define SCHEMA_NAMES_RESERVED_H

#include "dbc_schema.h"

static const DbcFieldDef names_reserved_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "Name",       DbcFieldType::String },
    { "Language",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_names_reserved = {
    "NamesReserved",
    names_reserved_fields,
    sizeof(names_reserved_fields) / sizeof(names_reserved_fields[0])
};

#endif
