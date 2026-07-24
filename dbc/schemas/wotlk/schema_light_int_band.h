#ifndef SCHEMA_LIGHT_INT_BAND_H
#define SCHEMA_LIGHT_INT_BAND_H

#include "dbc_schema.h"

static const DbcFieldDef light_int_band_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Num", DbcFieldType::UInt32 },
    { "Time1", DbcFieldType::UInt32 },
    { "Time2", DbcFieldType::UInt32 },
    { "Time3", DbcFieldType::UInt32 },
    { "Time4", DbcFieldType::UInt32 },
    { "Time5", DbcFieldType::UInt32 },
    { "Time6", DbcFieldType::UInt32 },
    { "Time7", DbcFieldType::UInt32 },
    { "Time8", DbcFieldType::UInt32 },
    { "Time9", DbcFieldType::UInt32 },
    { "Time10", DbcFieldType::UInt32 },
    { "Time11", DbcFieldType::UInt32 },
    { "Time12", DbcFieldType::UInt32 },
    { "Time13", DbcFieldType::UInt32 },
    { "Time14", DbcFieldType::UInt32 },
    { "Time15", DbcFieldType::UInt32 },
    { "Time16", DbcFieldType::UInt32 },
    { "Data1", DbcFieldType::UInt32 },
    { "Data2", DbcFieldType::UInt32 },
    { "Data3", DbcFieldType::UInt32 },
    { "Data4", DbcFieldType::UInt32 },
    { "Data5", DbcFieldType::UInt32 },
    { "Data6", DbcFieldType::UInt32 },
    { "Data7", DbcFieldType::UInt32 },
    { "Data8", DbcFieldType::UInt32 },
    { "Data9", DbcFieldType::UInt32 },
    { "Data10", DbcFieldType::UInt32 },
    { "Data11", DbcFieldType::UInt32 },
    { "Data12", DbcFieldType::UInt32 },
    { "Data13", DbcFieldType::UInt32 },
    { "Data14", DbcFieldType::UInt32 },
    { "Data15", DbcFieldType::UInt32 },
    { "Data16", DbcFieldType::UInt32 },
};

static const DbcSchema schema_light_int_band = {
    "LightIntBand",
    light_int_band_fields,
    sizeof(light_int_band_fields) / sizeof(light_int_band_fields[0])
};

#endif
