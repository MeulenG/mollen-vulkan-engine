#ifndef SCHEMA_DUNGEON_MAP_CHUNK_H
#define SCHEMA_DUNGEON_MAP_CHUNK_H

#include "dbc_schema.h"

static const DbcFieldDef dungeon_map_chunk_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "WmoGroupID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "wmogroup" },
    { "DungeonMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "dungeonmap" },
    { "MinZ", DbcFieldType::Float },
};

static const DbcSchema schema_dungeon_map_chunk = {
    "DungeonMapChunk",
    dungeon_map_chunk_fields,
    sizeof(dungeon_map_chunk_fields) / sizeof(dungeon_map_chunk_fields[0])
};

#endif
