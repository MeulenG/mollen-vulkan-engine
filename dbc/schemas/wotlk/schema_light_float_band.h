#ifndef SCHEMA_LIGHT_FLOAT_BAND_H
#define SCHEMA_LIGHT_FLOAT_BAND_H

#include "dbc_schema.h"

static const DbcFieldDef light_float_band_fields[] = {
    { "Id",       DbcFieldType::UInt32 },
    { "Num",      DbcFieldType::UInt32 },
    { "Time1",    DbcFieldType::UInt32 },
    { "Time2",    DbcFieldType::UInt32 },
    { "Time3",    DbcFieldType::UInt32 },
    { "Time4",    DbcFieldType::UInt32 },
    { "Time5",    DbcFieldType::UInt32 },
    { "Time6",    DbcFieldType::UInt32 },
    { "Time7",    DbcFieldType::UInt32 },
    { "Time8",    DbcFieldType::UInt32 },
    { "Time9",    DbcFieldType::UInt32 },
    { "Time10",   DbcFieldType::UInt32 },
    { "Time11",   DbcFieldType::UInt32 },
    { "Time12",   DbcFieldType::UInt32 },
    { "Time13",   DbcFieldType::UInt32 },
    { "Time14",   DbcFieldType::UInt32 },
    { "Time15",   DbcFieldType::UInt32 },
    { "Time16",   DbcFieldType::UInt32 },
    { "Data1",    DbcFieldType::Float  },
    { "Data2",    DbcFieldType::Float  },
    { "Data3",    DbcFieldType::Float  },
    { "Data4",    DbcFieldType::Float  },
    { "Data5",    DbcFieldType::Float  },
    { "Data6",    DbcFieldType::Float  },
    { "Data7",    DbcFieldType::Float  },
    { "Data8",    DbcFieldType::Float  },
    { "Data9",    DbcFieldType::Float  },
    { "Data10",   DbcFieldType::Float  },
    { "Data11",   DbcFieldType::Float  },
    { "Data12",   DbcFieldType::Float  },
    { "Data13",   DbcFieldType::Float  },
    { "Data14",   DbcFieldType::Float  },
    { "Data15",   DbcFieldType::Float  },
    { "Data16",   DbcFieldType::Float  },
};

static const DbcSchema schema_light_float_band = {
    "LightFloatBand",
    light_float_band_fields,
    sizeof(light_float_band_fields) / sizeof(light_float_band_fields[0])
};

#endif
