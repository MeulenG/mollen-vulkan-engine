#ifndef SCHEMA_ITEM_PURCHASE_GROUP_H
#define SCHEMA_ITEM_PURCHASE_GROUP_H

#include "dbc_schema.h"

static const DbcFieldDef item_purchase_group_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ItemID1", DbcFieldType::UInt32 },
    { "ItemID2", DbcFieldType::UInt32 },
    { "ItemID3", DbcFieldType::UInt32 },
    { "ItemID4", DbcFieldType::UInt32 },
    { "ItemID5", DbcFieldType::UInt32 },
    { "ItemID6", DbcFieldType::UInt32 },
    { "ItemID7", DbcFieldType::UInt32 },
    { "ItemID8", DbcFieldType::UInt32 },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
};

static const DbcSchema schema_item_purchase_group = {
    "ItemPurchaseGroup",
    item_purchase_group_fields,
    sizeof(item_purchase_group_fields) / sizeof(item_purchase_group_fields[0])
};

#endif
