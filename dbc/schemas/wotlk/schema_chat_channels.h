#ifndef SCHEMA_CHAT_CHANNELS_H
#define SCHEMA_CHAT_CHANNELS_H

#include "dbc_schema.h"

static const DbcFieldDef chat_channels_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Flags",         DbcFieldType::UInt32 },
    { "FactionGroup",  DbcFieldType::UInt32 },
    { "Name_enUS",     DbcFieldType::String },
    { "Name_koKR",     DbcFieldType::String },
    { "Name_frFR",     DbcFieldType::String },
    { "Name_deDE",     DbcFieldType::String },
    { "Name_enCN",     DbcFieldType::String },
    { "Name_zhTW",     DbcFieldType::String },
    { "Name_esES",     DbcFieldType::String },
    { "Name_esMX",     DbcFieldType::String },
    { "Name_ruRU",     DbcFieldType::String },
    { "Name_jaJP",     DbcFieldType::String },
    { "Name_ptPT",     DbcFieldType::String },
    { "Name_itIT",     DbcFieldType::String },
    { "Name_Unk1",     DbcFieldType::String },
    { "Name_Unk2",     DbcFieldType::String },
    { "Name_Unk3",     DbcFieldType::String },
    { "Name_Unk4",     DbcFieldType::String },
    { "NameFlags",     DbcFieldType::UInt32 },
    { "Shortcut_enUS", DbcFieldType::String },
    { "Shortcut_koKR", DbcFieldType::String },
    { "Shortcut_frFR", DbcFieldType::String },
    { "Shortcut_deDE", DbcFieldType::String },
    { "Shortcut_enCN", DbcFieldType::String },
    { "Shortcut_zhTW", DbcFieldType::String },
    { "Shortcut_esES", DbcFieldType::String },
    { "Shortcut_esMX", DbcFieldType::String },
    { "Shortcut_ruRU", DbcFieldType::String },
    { "Shortcut_jaJP", DbcFieldType::String },
    { "Shortcut_ptPT", DbcFieldType::String },
    { "Shortcut_itIT", DbcFieldType::String },
    { "Shortcut_Unk1", DbcFieldType::String },
    { "Shortcut_Unk2", DbcFieldType::String },
    { "Shortcut_Unk3", DbcFieldType::String },
    { "Shortcut_Unk4", DbcFieldType::String },
    { "ShortcutFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_chat_channels = {
    "ChatChannels",
    chat_channels_fields,
    sizeof(chat_channels_fields) / sizeof(chat_channels_fields[0])
};

#endif
