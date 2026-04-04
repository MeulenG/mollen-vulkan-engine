#ifndef SCHEMA_LIGHT_H
#define SCHEMA_LIGHT_H

#include "dbc_schema.h"

static const DbcFieldDef light_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "ContinentID",     DbcFieldType::UInt32 },
    { "X",               DbcFieldType::Float  },
    { "Y",               DbcFieldType::Float  },
    { "Z",               DbcFieldType::Float  },
    { "FalloffStart",    DbcFieldType::Float  },
    { "FalloffEnd",      DbcFieldType::Float  },
    { "LightParamsID1",  DbcFieldType::UInt32 },
    { "LightParamsID2",  DbcFieldType::UInt32 },
    { "LightParamsID3",  DbcFieldType::UInt32 },
    { "LightParamsID4",  DbcFieldType::UInt32 },
    { "LightParamsID5",  DbcFieldType::UInt32 },
    { "LightParamsID6",  DbcFieldType::UInt32 },
    { "LightParamsID7",  DbcFieldType::UInt32 },
    { "LightParamsID8",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_light = {
    "Light",
    light_fields,
    sizeof(light_fields) / sizeof(light_fields[0])
};

#endif
