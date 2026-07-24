#ifndef SCHEMA_LOADING_SCREEN_TAXI_SPLINES_H
#define SCHEMA_LOADING_SCREEN_TAXI_SPLINES_H

#include "dbc_schema.h"

static const DbcFieldDef loading_screen_taxi_splines_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "PathID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "path" },
    { "Leg1X1",      DbcFieldType::Float  },
    { "Leg1Y1",      DbcFieldType::Float  },
    { "Leg1X2",      DbcFieldType::Float  },
    { "Leg1Y2",      DbcFieldType::Float  },
    { "Leg1X3",      DbcFieldType::Float  },
    { "Leg1Y3",      DbcFieldType::Float  },
    { "Leg1X4",      DbcFieldType::Float  },
    { "Leg1Y4",      DbcFieldType::Float  },
    { "Leg2X1",      DbcFieldType::Float  },
    { "Leg2Y1",      DbcFieldType::Float  },
    { "Leg2X2",      DbcFieldType::Float  },
    { "Leg2Y2",      DbcFieldType::Float  },
    { "Leg2X3",      DbcFieldType::Float  },
    { "Leg2Y3",      DbcFieldType::Float  },
    { "Leg2X4",      DbcFieldType::Float  },
    { "Leg2Y4",      DbcFieldType::Float  },
    { "Leg2Count",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_loading_screen_taxi_splines = {
    "LoadingScreenTaxiSplines",
    loading_screen_taxi_splines_fields,
    sizeof(loading_screen_taxi_splines_fields) / sizeof(loading_screen_taxi_splines_fields[0])
};

#endif
