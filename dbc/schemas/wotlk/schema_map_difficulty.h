#ifndef SCHEMA_MAP_DIFFICULTY_H
#define SCHEMA_MAP_DIFFICULTY_H

#include "dbc_schema.h"

static const DbcFieldDef map_difficulty_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "MapID",         DbcFieldType::UInt32 },
    { "Difficulty",    DbcFieldType::UInt32 },
    { "Message_enUS",  DbcFieldType::String },
    { "Message_koKR",  DbcFieldType::String },
    { "Message_frFR",  DbcFieldType::String },
    { "Message_deDE",  DbcFieldType::String },
    { "Message_enCN",  DbcFieldType::String },
    { "Message_zhTW",  DbcFieldType::String },
    { "Message_esES",  DbcFieldType::String },
    { "Message_esMX",  DbcFieldType::String },
    { "Message_ruRU",  DbcFieldType::String },
    { "Message_jaJP",  DbcFieldType::String },
    { "Message_ptPT",  DbcFieldType::String },
    { "Message_itIT",  DbcFieldType::String },
    { "Message_Unk1",  DbcFieldType::String },
    { "Message_Unk2",  DbcFieldType::String },
    { "Message_Unk3",  DbcFieldType::String },
    { "Message_Unk4",  DbcFieldType::String },
    { "MessageFlags",  DbcFieldType::UInt32 },
    { "RaidDuration",  DbcFieldType::UInt32 },
    { "MaxPlayers",    DbcFieldType::UInt32 },
    { "Difficultystring", DbcFieldType::String },
};

static const DbcSchema schema_map_difficulty = {
    "MapDifficulty",
    map_difficulty_fields,
    sizeof(map_difficulty_fields) / sizeof(map_difficulty_fields[0])
};

#endif
