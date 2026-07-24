#ifndef SCHEMA_WOW_ERROR_STRINGS_H
#define SCHEMA_WOW_ERROR_STRINGS_H

#include "dbc_schema.h"

static const DbcFieldDef wow_error_strings_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Error_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "Error_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Error", "Identity" },
    { "ErrorFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_wow_error_strings = {
    "WowError_Strings",
    wow_error_strings_fields,
    sizeof(wow_error_strings_fields) / sizeof(wow_error_strings_fields[0])
};

#endif
