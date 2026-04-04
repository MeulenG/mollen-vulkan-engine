#ifndef SCHEMA_LIGHT_PARAMS_H
#define SCHEMA_LIGHT_PARAMS_H

#include "dbc_schema.h"

static const DbcFieldDef light_params_fields[] = {
    { "Id",                 DbcFieldType::UInt32 },
    { "HighlightSky",       DbcFieldType::UInt32 },
    { "LightSkyboxID",      DbcFieldType::UInt32 },
    { "CloudTypeID",        DbcFieldType::UInt32 },
    { "Glow",               DbcFieldType::Float  },
    { "WaterShallowAlpha",  DbcFieldType::Float  },
    { "WaterDeepAlpha",     DbcFieldType::Float  },
    { "OceanShallowAlpha",  DbcFieldType::Float  },
    { "OceanDeepAlpha",     DbcFieldType::Float  },
};

static const DbcSchema schema_light_params = {
    "LightParams",
    light_params_fields,
    sizeof(light_params_fields) / sizeof(light_params_fields[0])
};

#endif
