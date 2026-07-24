#ifndef SCHEMA_WORLD_CHUNK_SOUNDS_H
#define SCHEMA_WORLD_CHUNK_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef world_chunk_sounds_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "WorldMapAreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldmaparea" },
    { "ChunkX", DbcFieldType::UInt32 },
    { "ChunkY", DbcFieldType::UInt32 },
    { "SubchunkX", DbcFieldType::UInt32 },
    { "SubchunkY", DbcFieldType::UInt32 },
    { "ZoneIntroMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zoneintromusic", "Visuals" },
    { "ZoneMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zonemusic", "Visuals" },
    { "SoundAmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundambience", "Visuals" },
};

static const DbcSchema schema_world_chunk_sounds = {
    "WorldChunkSounds",
    world_chunk_sounds_fields,
    sizeof(world_chunk_sounds_fields) / sizeof(world_chunk_sounds_fields[0])
};

#endif
