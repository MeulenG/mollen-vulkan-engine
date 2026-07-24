#ifndef SCHEMA_ITEM_BAG_FAMILY_H
#define SCHEMA_ITEM_BAG_FAMILY_H

#include "dbc_schema.h"

static const DbcFieldDef item_bag_family_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
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

static const DbcSchema schema_item_bag_family = {
    "ItemBagFamily",
    item_bag_family_fields,
    sizeof(item_bag_family_fields) / sizeof(item_bag_family_fields[0])
};

#endif
