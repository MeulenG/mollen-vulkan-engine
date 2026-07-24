#ifndef SCHEMA_TAXI_PATH_H
#define SCHEMA_TAXI_PATH_H

#include "dbc_schema.h"

static const DbcFieldDef taxi_path_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "From", DbcFieldType::UInt32 },
    { "To", DbcFieldType::UInt32 },
    { "Cost", DbcFieldType::UInt32 },
};

static const DbcSchema schema_taxi_path = {
    "TaxiPath",
    taxi_path_fields,
    sizeof(taxi_path_fields) / sizeof(taxi_path_fields[0])
};

#endif
