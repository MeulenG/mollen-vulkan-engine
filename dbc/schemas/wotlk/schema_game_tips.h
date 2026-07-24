#ifndef SCHEMA_GAME_TIPS_H
#define SCHEMA_GAME_TIPS_H

#include "dbc_schema.h"

static const DbcFieldDef game_tips_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Text_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "Text_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Text" },
    { "TextFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_game_tips = {
    "GameTips",
    game_tips_fields,
    sizeof(game_tips_fields) / sizeof(game_tips_fields[0])
};

#endif
