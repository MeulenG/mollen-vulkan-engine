#ifndef SCHEMA_DUNGEON_MAP_CHUNK_H
#define SCHEMA_DUNGEON_MAP_CHUNK_H

#include "dbc_schema.h"

static const DbcFieldDef dungeon_map_chunk_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "MapID",          DbcFieldType::UInt32 },
    { "WmoGroupID",     DbcFieldType::UInt32 },
    { "DungeonMapID",   DbcFieldType::UInt32 },
    { "MinZ",           DbcFieldType::Float  },
};

static const DbcSchema schema_dungeon_map_chunk = {
    "DungeonMapChunk",
    dungeon_map_chunk_fields,
    sizeof(dungeon_map_chunk_fields) / sizeof(dungeon_map_chunk_fields[0])
};

#endif
