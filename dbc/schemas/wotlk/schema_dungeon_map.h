#ifndef SCHEMA_DUNGEON_MAP_H
#define SCHEMA_DUNGEON_MAP_H

#include "dbc_schema.h"

static const DbcFieldDef dungeon_map_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "MapID",         DbcFieldType::UInt32 },
    { "FloorIndex",    DbcFieldType::UInt32 },
    { "MinX",          DbcFieldType::Float  },
    { "MaxX",          DbcFieldType::Float  },
    { "MinY",          DbcFieldType::Float  },
    { "MaxY",          DbcFieldType::Float  },
    { "ParentWorldMapID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_dungeon_map = {
    "DungeonMap",
    dungeon_map_fields,
    sizeof(dungeon_map_fields) / sizeof(dungeon_map_fields[0])
};

#endif
