#ifndef SCHEMA_HOLIDAY_DESCRIPTIONS_H
#define SCHEMA_HOLIDAY_DESCRIPTIONS_H

#include "dbc_schema.h"

static const DbcFieldDef holiday_descriptions_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Description_enUS", DbcFieldType::String },
    { "Description_koKR", DbcFieldType::String },
    { "Description_frFR", DbcFieldType::String },
    { "Description_deDE", DbcFieldType::String },
    { "Description_enCN", DbcFieldType::String },
    { "Description_zhTW", DbcFieldType::String },
    { "Description_esES", DbcFieldType::String },
    { "Description_esMX", DbcFieldType::String },
    { "Description_ruRU", DbcFieldType::String },
    { "Description_jaJP", DbcFieldType::String },
    { "Description_ptPT", DbcFieldType::String },
    { "Description_itIT", DbcFieldType::String },
    { "Description_Unk1", DbcFieldType::String },
    { "Description_Unk2", DbcFieldType::String },
    { "Description_Unk3", DbcFieldType::String },
    { "Description_Unk4", DbcFieldType::String },
    { "DescriptionFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_holiday_descriptions = {
    "HolidayDescriptions",
    holiday_descriptions_fields,
    sizeof(holiday_descriptions_fields) / sizeof(holiday_descriptions_fields[0])
};

#endif
