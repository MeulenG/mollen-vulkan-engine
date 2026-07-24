#ifndef SCHEMA_NAME_GEN_H
#define SCHEMA_NAME_GEN_H

#include "dbc_schema.h"

static const DbcFieldDef name_gen_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "Sex", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
};

static const DbcSchema schema_name_gen = {
    "NameGen",
    name_gen_fields,
    sizeof(name_gen_fields) / sizeof(name_gen_fields[0])
};

#endif
