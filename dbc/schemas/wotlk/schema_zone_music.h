#ifndef SCHEMA_ZONE_MUSIC_H
#define SCHEMA_ZONE_MUSIC_H

#include "dbc_schema.h"

static const DbcFieldDef zone_music_fields[] = {
    { "Id",                DbcFieldType::UInt32 },
    { "SetName",           DbcFieldType::String },
    { "SilenceIntervalDay",DbcFieldType::UInt32 },
    { "SilenceIntervalNight",DbcFieldType::UInt32 },
    { "SoundsDay",         DbcFieldType::UInt32 },
    { "SoundsNight",       DbcFieldType::UInt32 },
    { "MinDelayDay",       DbcFieldType::UInt32 },
    { "MinDelayNight",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_zone_music = {
    "ZoneMusic",
    zone_music_fields,
    sizeof(zone_music_fields) / sizeof(zone_music_fields[0])
};

#endif
