#ifndef SCHEMA_WORLD_STATE_ZONE_SOUNDS_H
#define SCHEMA_WORLD_STATE_ZONE_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef world_state_zone_sounds_fields[] = {
    { "Id",                 DbcFieldType::UInt32 },
    { "WorldStateID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldstate" },
    { "WorldStateValue",    DbcFieldType::UInt32 },
    { "AreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "areatable" },
    { "WMOAreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "wmoarea" },
    { "ZoneIntroMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zoneintromusic" },
    { "ZoneMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zonemusic" },
    { "SoundAmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundambience" },
};

static const DbcSchema schema_world_state_zone_sounds = {
    "WorldStateZoneSounds",
    world_state_zone_sounds_fields,
    sizeof(world_state_zone_sounds_fields) / sizeof(world_state_zone_sounds_fields[0])
};

#endif
