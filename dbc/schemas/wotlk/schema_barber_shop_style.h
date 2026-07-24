#ifndef SCHEMA_BARBER_SHOP_STYLE_H
#define SCHEMA_BARBER_SHOP_STYLE_H

#include "dbc_schema.h"

static const DbcFieldDef barber_shop_style_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Type",            DbcFieldType::UInt32 },
    { "DisplayName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Description_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "DescriptionFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "CostModifier",   DbcFieldType::Float  },
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "SexID", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
    { "Data",            DbcFieldType::UInt32 },
};

static const DbcSchema schema_barber_shop_style = {
    "BarberShopStyle",
    barber_shop_style_fields,
    sizeof(barber_shop_style_fields) / sizeof(barber_shop_style_fields[0])
};

#endif
