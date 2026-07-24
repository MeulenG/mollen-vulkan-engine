#ifndef SCHEMA_POWER_DISPLAY_H
#define SCHEMA_POWER_DISPLAY_H

#include "dbc_schema.h"

// Record size: 15 bytes - no trailing padding
static const DbcFieldDef power_display_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ActualType", DbcFieldType::UInt32 },
    { "GlobalStringBaseTag", DbcFieldType::String },
    { "Red", DbcFieldType::UInt8 },
    { "Green", DbcFieldType::UInt8 },
    { "Blue", DbcFieldType::UInt8 },
};

static const DbcSchema schema_power_display = {
    "PowerDisplay",
    power_display_fields,
    sizeof(power_display_fields) / sizeof(power_display_fields[0]),
    true
};

#endif
