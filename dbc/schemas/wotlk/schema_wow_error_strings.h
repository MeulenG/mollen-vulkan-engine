#ifndef SCHEMA_WOW_ERROR_STRINGS_H
#define SCHEMA_WOW_ERROR_STRINGS_H

#include "dbc_schema.h"

static const DbcFieldDef wow_error_strings_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Name",          DbcFieldType::String },
    { "Error_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "Error_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Error" },
    { "ErrorFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_wow_error_strings = {
    "WowError_Strings",
    wow_error_strings_fields,
    sizeof(wow_error_strings_fields) / sizeof(wow_error_strings_fields[0])
};

#endif
