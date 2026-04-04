#ifndef SCHEMA_TRANSPORT_ANIMATION_H
#define SCHEMA_TRANSPORT_ANIMATION_H

#include "dbc_schema.h"

static const DbcFieldDef transport_animation_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "TransportID",    DbcFieldType::UInt32 },
    { "TimeIndex",      DbcFieldType::UInt32 },
    { "PosX",           DbcFieldType::Float  },
    { "PosY",           DbcFieldType::Float  },
    { "PosZ",           DbcFieldType::Float  },
    { "SequenceID",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_transport_animation = {
    "TransportAnimation",
    transport_animation_fields,
    sizeof(transport_animation_fields) / sizeof(transport_animation_fields[0])
};

#endif
