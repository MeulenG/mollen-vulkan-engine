#ifndef SCHEMA_CHAR_BASE_INFO_H
#define SCHEMA_CHAR_BASE_INFO_H

#include "dbc_schema.h"

// Record size: 2 bytes — no Id field, no padding
// Each record is just a valid race+class combination
static const DbcFieldDef char_base_info_fields[] = {
    { "RaceID",    DbcFieldType::UInt8 },
    { "ClassID",   DbcFieldType::UInt8 },
};

static const DbcSchema schema_char_base_info = {
    "CharBaseInfo",
    char_base_info_fields,
    sizeof(char_base_info_fields) / sizeof(char_base_info_fields[0]),
    true
};

#endif
