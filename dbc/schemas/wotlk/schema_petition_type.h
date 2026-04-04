#ifndef SCHEMA_PETITION_TYPE_H
#define SCHEMA_PETITION_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef petition_type_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "Name",        DbcFieldType::String },
    { "Type",        DbcFieldType::UInt32 },
};

static const DbcSchema schema_petition_type = {
    "PetitionType",
    petition_type_fields,
    sizeof(petition_type_fields) / sizeof(petition_type_fields[0])
};

#endif
