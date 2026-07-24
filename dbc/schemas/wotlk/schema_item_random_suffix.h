#ifndef SCHEMA_ITEM_RANDOM_SUFFIX_H
#define SCHEMA_ITEM_RANDOM_SUFFIX_H

#include "dbc_schema.h"

static const DbcFieldDef item_random_suffix_fields[] = {
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
    { "InternalName", DbcFieldType::String },
    { "Enchantment1", DbcFieldType::UInt32 },
    { "Enchantment2", DbcFieldType::UInt32 },
    { "Enchantment3", DbcFieldType::UInt32 },
    { "Enchantment4", DbcFieldType::UInt32 },
    { "Enchantment5", DbcFieldType::UInt32 },
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
