#ifndef SCHEMA_AREA_TRIGGER_H
#define SCHEMA_AREA_TRIGGER_H

#include "dbc_schema.h"

static const DbcFieldDef area_trigger_fields[] = {
    { "Id",      DbcFieldType::UInt32 },
    { "MapID",   DbcFieldType::UInt32 },
    { "X",       DbcFieldType::Float  },
    { "Y",       DbcFieldType::Float  },
    { "Z",       DbcFieldType::Float  },
    { "Radius",  DbcFieldType::Float  },
    { "BoxLength", DbcFieldType::Float },
    { "BoxWidth", DbcFieldType::Float  },
    { "BoxHeight", DbcFieldType::Float },
    { "BoxYaw",  DbcFieldType::Float  },
};

static const DbcSchema schema_area_trigger = {
    "AreaTrigger",
    area_trigger_fields,
    sizeof(area_trigger_fields) / sizeof(area_trigger_fields[0])
};

#endif
