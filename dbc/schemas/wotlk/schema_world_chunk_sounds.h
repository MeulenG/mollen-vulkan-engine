#ifndef SCHEMA_WORLD_CHUNK_SOUNDS_H
#define SCHEMA_WORLD_CHUNK_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef world_chunk_sounds_fields[] = {
    { "Id",                DbcFieldType::UInt32 },
    { "WorldMapAreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldmaparea" },
    { "ChunkX",            DbcFieldType::UInt32 },
    { "ChunkY",            DbcFieldType::UInt32 },
    { "SubchunkX",         DbcFieldType::UInt32 },
    { "SubchunkY",         DbcFieldType::UInt32 },
    { "ZoneIntroMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zoneintromusic" },
    { "ZoneMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zonemusic" },
    { "SoundAmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundambience" },
};

static const DbcSchema schema_world_chunk_sounds = {
    "WorldChunkSounds",
    world_chunk_sounds_fields,
    sizeof(world_chunk_sounds_fields) / sizeof(world_chunk_sounds_fields[0])
};

#endif
