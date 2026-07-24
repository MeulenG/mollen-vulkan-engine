#ifndef SCHEMA_TAXI_PATH_NODE_H
#define SCHEMA_TAXI_PATH_NODE_H

#include "dbc_schema.h"

static const DbcFieldDef taxi_path_node_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "PathID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "path" },
    { "NodeIndex",      DbcFieldType::UInt32 },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "X",              DbcFieldType::Float  },
    { "Y",              DbcFieldType::Float  },
    { "Z",              DbcFieldType::Float  },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Delay",          DbcFieldType::UInt32 },
    { "ArrivalEventID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "arrivalevent" },
    { "DepartureEventID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "departureevent" },
};

static const DbcSchema schema_taxi_path_node = {
    "TaxiPathNode",
    taxi_path_node_fields,
    sizeof(taxi_path_node_fields) / sizeof(taxi_path_node_fields[0])
};

#endif
