#ifndef SCHEMA_ITEM_PURCHASE_GROUP_H
#define SCHEMA_ITEM_PURCHASE_GROUP_H

#include "dbc_schema.h"

static const DbcFieldDef item_purchase_group_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "ItemID1",       DbcFieldType::UInt32 },
    { "ItemID2",       DbcFieldType::UInt32 },
    { "ItemID3",       DbcFieldType::UInt32 },
    { "ItemID4",       DbcFieldType::UInt32 },
    { "ItemID5",       DbcFieldType::UInt32 },
    { "ItemID6",       DbcFieldType::UInt32 },
    { "ItemID7",       DbcFieldType::UInt32 },
    { "ItemID8",       DbcFieldType::UInt32 },
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
};

static const DbcSchema schema_item_purchase_group = {
    "ItemPurchaseGroup",
    item_purchase_group_fields,
    sizeof(item_purchase_group_fields) / sizeof(item_purchase_group_fields[0])
};

#endif
