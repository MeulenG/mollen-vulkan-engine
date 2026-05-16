#ifndef SCHEMA_ITEM_RANDOM_PROPERTIES_H
#define SCHEMA_ITEM_RANDOM_PROPERTIES_H

#include "dbc_schema.h"

static const DbcFieldDef item_random_properties_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Enchantment1", DbcFieldType::UInt32 },
    { "Enchantment2", DbcFieldType::UInt32 },
    { "Enchantment3", DbcFieldType::UInt32 },
    { "Enchantment4", DbcFieldType::UInt32 },
    { "Enchantment5", DbcFieldType::UInt32 },
    { "Suffix_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "Suffix_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix", "Identity" },
    { "SuffixFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_item_random_properties = {
    "ItemRandomProperties",
    item_random_properties_fields,
    sizeof(item_random_properties_fields) / sizeof(item_random_properties_fields[0])
};

#endif
