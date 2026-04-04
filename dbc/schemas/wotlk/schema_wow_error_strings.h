#ifndef SCHEMA_WOW_ERROR_STRINGS_H
#define SCHEMA_WOW_ERROR_STRINGS_H

#include "dbc_schema.h"

static const DbcFieldDef wow_error_strings_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Name",          DbcFieldType::String },
    { "Error_enUS",    DbcFieldType::String },
    { "Error_koKR",    DbcFieldType::String },
    { "Error_frFR",    DbcFieldType::String },
    { "Error_deDE",    DbcFieldType::String },
    { "Error_enCN",    DbcFieldType::String },
    { "Error_zhTW",    DbcFieldType::String },
    { "Error_esES",    DbcFieldType::String },
    { "Error_esMX",    DbcFieldType::String },
    { "Error_ruRU",    DbcFieldType::String },
    { "Error_jaJP",    DbcFieldType::String },
    { "Error_ptPT",    DbcFieldType::String },
    { "Error_itIT",    DbcFieldType::String },
    { "Error_Unk1",    DbcFieldType::String },
    { "Error_Unk2",    DbcFieldType::String },
    { "Error_Unk3",    DbcFieldType::String },
    { "Error_Unk4",    DbcFieldType::String },
    { "ErrorFlags",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_wow_error_strings = {
    "WowError_Strings",
    wow_error_strings_fields,
    sizeof(wow_error_strings_fields) / sizeof(wow_error_strings_fields[0])
};

#endif
