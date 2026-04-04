#ifndef SCHEMA_ITEM_RANDOM_SUFFIX_H
#define SCHEMA_ITEM_RANDOM_SUFFIX_H

#include "dbc_schema.h"

static const DbcFieldDef item_random_suffix_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "Name_enUS",      DbcFieldType::String },
    { "Name_koKR",      DbcFieldType::String },
    { "Name_frFR",      DbcFieldType::String },
    { "Name_deDE",      DbcFieldType::String },
    { "Name_enCN",      DbcFieldType::String },
    { "Name_zhTW",      DbcFieldType::String },
    { "Name_esES",      DbcFieldType::String },
    { "Name_esMX",      DbcFieldType::String },
    { "Name_ruRU",      DbcFieldType::String },
    { "Name_jaJP",      DbcFieldType::String },
    { "Name_ptPT",      DbcFieldType::String },
    { "Name_itIT",      DbcFieldType::String },
    { "Name_Unk1",      DbcFieldType::String },
    { "Name_Unk2",      DbcFieldType::String },
    { "Name_Unk3",      DbcFieldType::String },
    { "Name_Unk4",      DbcFieldType::String },
    { "NameFlags",      DbcFieldType::UInt32 },
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
