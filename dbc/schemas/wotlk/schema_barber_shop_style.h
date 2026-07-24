#ifndef SCHEMA_BARBER_SHOP_STYLE_H
#define SCHEMA_BARBER_SHOP_STYLE_H

#include "dbc_schema.h"

static const DbcFieldDef barber_shop_style_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Type", DbcFieldType::UInt32 },
    { "DisplayName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
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
    { "CostModifier", DbcFieldType::Float },
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "SexID", DbcFieldType::UInt32, DbcSemantic::Enum, "Sex" },
    { "Data", DbcFieldType::UInt32 },
};

static const DbcSchema schema_barber_shop_style = {
    "BarberShopStyle",
    barber_shop_style_fields,
    sizeof(barber_shop_style_fields) / sizeof(barber_shop_style_fields[0])
};

#endif
