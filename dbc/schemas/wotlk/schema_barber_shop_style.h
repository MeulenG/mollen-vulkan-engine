#ifndef SCHEMA_BARBER_SHOP_STYLE_H
#define SCHEMA_BARBER_SHOP_STYLE_H

#include "dbc_schema.h"

static const DbcFieldDef barber_shop_style_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Type",            DbcFieldType::UInt32 },
    { "DisplayName_enUS", DbcFieldType::String },
    { "DisplayName_koKR", DbcFieldType::String },
    { "DisplayName_frFR", DbcFieldType::String },
    { "DisplayName_deDE", DbcFieldType::String },
    { "DisplayName_enCN", DbcFieldType::String },
    { "DisplayName_zhTW", DbcFieldType::String },
    { "DisplayName_esES", DbcFieldType::String },
    { "DisplayName_esMX", DbcFieldType::String },
    { "DisplayName_ruRU", DbcFieldType::String },
    { "DisplayName_jaJP", DbcFieldType::String },
    { "DisplayName_ptPT", DbcFieldType::String },
    { "DisplayName_itIT", DbcFieldType::String },
    { "DisplayName_Unk1", DbcFieldType::String },
    { "DisplayName_Unk2", DbcFieldType::String },
    { "DisplayName_Unk3", DbcFieldType::String },
    { "DisplayName_Unk4", DbcFieldType::String },
    { "DisplayNameFlags", DbcFieldType::UInt32 },
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
    { "CostModifier",   DbcFieldType::Float  },
    { "RaceID",          DbcFieldType::UInt32 },
    { "SexID",           DbcFieldType::UInt32 },
    { "Data",            DbcFieldType::UInt32 },
};

static const DbcSchema schema_barber_shop_style = {
    "BarberShopStyle",
    barber_shop_style_fields,
    sizeof(barber_shop_style_fields) / sizeof(barber_shop_style_fields[0])
};

#endif
