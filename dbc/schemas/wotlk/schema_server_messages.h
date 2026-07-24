#ifndef SCHEMA_SERVER_MESSAGES_H
#define SCHEMA_SERVER_MESSAGES_H

#include "dbc_schema.h"

static const DbcFieldDef server_messages_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Text_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "Text_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Text", "Identity" },
    { "TextFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_server_messages = {
    "ServerMessages",
    server_messages_fields,
    sizeof(server_messages_fields) / sizeof(server_messages_fields[0])
};

#endif
