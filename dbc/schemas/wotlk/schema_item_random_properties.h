#ifndef SCHEMA_ITEM_RANDOM_PROPERTIES_H
#define SCHEMA_ITEM_RANDOM_PROPERTIES_H

#include "dbc_schema.h"

static const DbcFieldDef item_random_properties_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Name",          DbcFieldType::String },
    { "Enchantment1",  DbcFieldType::UInt32 },
    { "Enchantment2",  DbcFieldType::UInt32 },
    { "Enchantment3",  DbcFieldType::UInt32 },
    { "Enchantment4",  DbcFieldType::UInt32 },
    { "Enchantment5",  DbcFieldType::UInt32 },
    { "Suffix_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "Suffix_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Suffix" },
    { "SuffixFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_item_random_properties = {
    "ItemRandomProperties",
    item_random_properties_fields,
    sizeof(item_random_properties_fields) / sizeof(item_random_properties_fields[0])
};

#endif
