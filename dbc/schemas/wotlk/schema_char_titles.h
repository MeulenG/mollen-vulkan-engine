#ifndef SCHEMA_CHAR_TITLES_H
#define SCHEMA_CHAR_TITLES_H

#include "dbc_schema.h"

static const DbcFieldDef char_titles_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "ConditionID",   DbcFieldType::UInt32 },
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
    { "NameFemale_enUS", DbcFieldType::String },
    { "NameFemale_koKR", DbcFieldType::String },
    { "NameFemale_frFR", DbcFieldType::String },
    { "NameFemale_deDE", DbcFieldType::String },
    { "NameFemale_enCN", DbcFieldType::String },
    { "NameFemale_zhTW", DbcFieldType::String },
    { "NameFemale_esES", DbcFieldType::String },
    { "NameFemale_esMX", DbcFieldType::String },
    { "NameFemale_ruRU", DbcFieldType::String },
    { "NameFemale_jaJP", DbcFieldType::String },
    { "NameFemale_ptPT", DbcFieldType::String },
    { "NameFemale_itIT", DbcFieldType::String },
    { "NameFemale_Unk1", DbcFieldType::String },
    { "NameFemale_Unk2", DbcFieldType::String },
    { "NameFemale_Unk3", DbcFieldType::String },
    { "NameFemale_Unk4", DbcFieldType::String },
    { "NameFemaleFlags", DbcFieldType::UInt32 },
    { "MaskID",        DbcFieldType::UInt32 },
};

static const DbcSchema schema_char_titles = {
    "CharTitles",
    char_titles_fields,
    sizeof(char_titles_fields) / sizeof(char_titles_fields[0])
};

#endif
