#ifndef SCHEMA_HOLIDAY_DESCRIPTIONS_H
#define SCHEMA_HOLIDAY_DESCRIPTIONS_H

#include "dbc_schema.h"

static const DbcFieldDef holiday_descriptions_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Description_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "DescriptionFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
};

static const DbcSchema schema_holiday_descriptions = {
    "HolidayDescriptions",
    holiday_descriptions_fields,
    sizeof(holiday_descriptions_fields) / sizeof(holiday_descriptions_fields[0])
};

#endif
