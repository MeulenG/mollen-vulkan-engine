#ifndef SCHEMA_EMOTES_TEXT_DATA_H
#define SCHEMA_EMOTES_TEXT_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef emotes_text_data_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Text_enUS", DbcFieldType::String },
    { "Text_koKR", DbcFieldType::String },
    { "Text_frFR", DbcFieldType::String },
    { "Text_deDE", DbcFieldType::String },
    { "Text_enCN", DbcFieldType::String },
    { "Text_zhTW", DbcFieldType::String },
    { "Text_esES", DbcFieldType::String },
    { "Text_esMX", DbcFieldType::String },
    { "Text_ruRU", DbcFieldType::String },
    { "Text_jaJP", DbcFieldType::String },
    { "Text_ptPT", DbcFieldType::String },
    { "Text_itIT", DbcFieldType::String },
    { "Text_Unk1", DbcFieldType::String },
    { "Text_Unk2", DbcFieldType::String },
    { "Text_Unk3", DbcFieldType::String },
    { "Text_Unk4", DbcFieldType::String },
    { "TextFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_emotes_text_data = {
    "EmotesTextData",
    emotes_text_data_fields,
    sizeof(emotes_text_data_fields) / sizeof(emotes_text_data_fields[0])
};

#endif
