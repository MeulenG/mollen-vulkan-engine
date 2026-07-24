#ifndef SCHEMA_ZONE_INTRO_MUSIC_TABLE_H
#define SCHEMA_ZONE_INTRO_MUSIC_TABLE_H

#include "dbc_schema.h"

static const DbcFieldDef zone_intro_music_table_fields[] = {
    { "Id",               DbcFieldType::UInt32 },
    { "Name",             DbcFieldType::String },
    { "SoundEntriesID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries" },
    { "Priority",         DbcFieldType::UInt32 },
    { "MinDelayMinutes",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_zone_intro_music_table = {
    "ZoneIntroMusicTable",
    zone_intro_music_table_fields,
    sizeof(zone_intro_music_table_fields) / sizeof(zone_intro_music_table_fields[0])
};

#endif
