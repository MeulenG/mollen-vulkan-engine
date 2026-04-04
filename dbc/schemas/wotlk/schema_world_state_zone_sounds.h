#ifndef SCHEMA_WORLD_STATE_ZONE_SOUNDS_H
#define SCHEMA_WORLD_STATE_ZONE_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef world_state_zone_sounds_fields[] = {
    { "Id",                 DbcFieldType::UInt32 },
    { "WorldStateID",       DbcFieldType::UInt32 },
    { "WorldStateValue",    DbcFieldType::UInt32 },
    { "AreaID",             DbcFieldType::UInt32 },
    { "WMOAreaID",          DbcFieldType::UInt32 },
    { "ZoneIntroMusicID",   DbcFieldType::UInt32 },
    { "ZoneMusicID",        DbcFieldType::UInt32 },
    { "SoundAmbienceID",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_world_state_zone_sounds = {
    "WorldStateZoneSounds",
    world_state_zone_sounds_fields,
    sizeof(world_state_zone_sounds_fields) / sizeof(world_state_zone_sounds_fields[0])
};

#endif
