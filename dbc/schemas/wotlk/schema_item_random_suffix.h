#ifndef SCHEMA_ITEM_RANDOM_SUFFIX_H
#define SCHEMA_ITEM_RANDOM_SUFFIX_H

#include "dbc_schema.h"

static const DbcFieldDef item_random_suffix_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
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
    { "InternalName",   DbcFieldType::String },
    { "Enchantment1",   DbcFieldType::UInt32 },
    { "Enchantment2",   DbcFieldType::UInt32 },
    { "Enchantment3",   DbcFieldType::UInt32 },
    { "Enchantment4",   DbcFieldType::UInt32 },
    { "Enchantment5",   DbcFieldType::UInt32 },
    { "AllocationPct1", DbcFieldType::UInt32 },
    { "AllocationPct2", DbcFieldType::UInt32 },
    { "AllocationPct3", DbcFieldType::UInt32 },
    { "AllocationPct4", DbcFieldType::UInt32 },
    { "AllocationPct5", DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_random_suffix = {
    "ItemRandomSuffix",
    item_random_suffix_fields,
    sizeof(item_random_suffix_fields) / sizeof(item_random_suffix_fields[0])
};

#endif
