#ifndef SCHEMA_CHAT_CHANNELS_H
#define SCHEMA_CHAT_CHANNELS_H

#include "dbc_schema.h"

static const DbcFieldDef chat_channels_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "FactionGroup", DbcFieldType::UInt32 },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
    { "Shortcut_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "Shortcut_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Shortcut", "Identity" },
    { "ShortcutFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_chat_channels = {
    "ChatChannels",
    chat_channels_fields,
    sizeof(chat_channels_fields) / sizeof(chat_channels_fields[0])
};

#endif
