#ifndef SCHEMA_TRANSPORT_ROTATION_H
#define SCHEMA_TRANSPORT_ROTATION_H

#include "dbc_schema.h"

static const DbcFieldDef transport_rotation_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "GameObjectsID",  DbcFieldType::UInt32 },
    { "TimeIndex",      DbcFieldType::UInt32 },
    { "RotX",           DbcFieldType::Float  },
    { "RotY",           DbcFieldType::Float  },
    { "RotZ",           DbcFieldType::Float  },
    { "RotW",           DbcFieldType::Float  },
};

static const DbcSchema schema_transport_rotation = {
    "TransportRotation",
    transport_rotation_fields,
    sizeof(transport_rotation_fields) / sizeof(transport_rotation_fields[0])
};

#endif
