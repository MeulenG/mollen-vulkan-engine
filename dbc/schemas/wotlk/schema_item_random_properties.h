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
    { "Suffix_enUS",   DbcFieldType::String },
    { "Suffix_koKR",   DbcFieldType::String },
    { "Suffix_frFR",   DbcFieldType::String },
    { "Suffix_deDE",   DbcFieldType::String },
    { "Suffix_enCN",   DbcFieldType::String },
    { "Suffix_zhTW",   DbcFieldType::String },
    { "Suffix_esES",   DbcFieldType::String },
    { "Suffix_esMX",   DbcFieldType::String },
    { "Suffix_ruRU",   DbcFieldType::String },
    { "Suffix_jaJP",   DbcFieldType::String },
    { "Suffix_ptPT",   DbcFieldType::String },
    { "Suffix_itIT",   DbcFieldType::String },
    { "Suffix_Unk1",   DbcFieldType::String },
    { "Suffix_Unk2",   DbcFieldType::String },
    { "Suffix_Unk3",   DbcFieldType::String },
    { "Suffix_Unk4",   DbcFieldType::String },
    { "SuffixFlags",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_random_properties = {
    "ItemRandomProperties",
    item_random_properties_fields,
    sizeof(item_random_properties_fields) / sizeof(item_random_properties_fields[0])
};

#endif
