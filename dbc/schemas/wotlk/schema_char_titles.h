#ifndef SCHEMA_CHAR_TITLES_H
#define SCHEMA_CHAR_TITLES_H

#include "dbc_schema.h"

static const DbcFieldDef char_titles_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "ConditionID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "condition" },
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
    { "NameFemale_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemale_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "NameFemale" },
    { "NameFemaleFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "MaskID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "mask" },
};

static const DbcSchema schema_char_titles = {
    "CharTitles",
    char_titles_fields,
    sizeof(char_titles_fields) / sizeof(char_titles_fields[0])
};

#endif
