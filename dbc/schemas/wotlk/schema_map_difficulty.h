#ifndef SCHEMA_MAP_DIFFICULTY_H
#define SCHEMA_MAP_DIFFICULTY_H

#include "dbc_schema.h"

static const DbcFieldDef map_difficulty_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "Difficulty",    DbcFieldType::UInt32 },
    { "Message_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "Message_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Message" },
    { "MessageFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
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
