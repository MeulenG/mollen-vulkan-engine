#ifndef SCHEMA_CHAT_CHANNELS_H
#define SCHEMA_CHAT_CHANNELS_H

#include "dbc_schema.h"

static const DbcFieldDef chat_channels_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "FactionGroup",  DbcFieldType::UInt32 },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Shortcut_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "Shortcut_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut" },
    { "ShortcutFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_chat_channels = {
    "ChatChannels",
    chat_channels_fields,
    sizeof(chat_channels_fields) / sizeof(chat_channels_fields[0])
};

#endif
