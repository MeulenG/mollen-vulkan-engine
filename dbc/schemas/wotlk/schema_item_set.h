#ifndef SCHEMA_ITEM_SET_H
#define SCHEMA_ITEM_SET_H

#include "dbc_schema.h"

static const DbcFieldDef item_set_fields[] = {
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
    { "ItemID1", DbcFieldType::UInt32 },
    { "ItemID2", DbcFieldType::UInt32 },
    { "ItemID3", DbcFieldType::UInt32 },
    { "ItemID4", DbcFieldType::UInt32 },
    { "ItemID5", DbcFieldType::UInt32 },
    { "ItemID6", DbcFieldType::UInt32 },
    { "ItemID7", DbcFieldType::UInt32 },
    { "ItemID8", DbcFieldType::UInt32 },
    { "ItemID9", DbcFieldType::UInt32 },
    { "ItemID10", DbcFieldType::UInt32 },
    { "ItemID11", DbcFieldType::UInt32 },
    { "ItemID12", DbcFieldType::UInt32 },
    { "ItemID13", DbcFieldType::UInt32 },
    { "ItemID14", DbcFieldType::UInt32 },
    { "ItemID15", DbcFieldType::UInt32 },
    { "ItemID16", DbcFieldType::UInt32 },
    { "ItemID17", DbcFieldType::UInt32 },
    { "SetSpellID1", DbcFieldType::UInt32 },
    { "SetSpellID2", DbcFieldType::UInt32 },
    { "SetSpellID3", DbcFieldType::UInt32 },
    { "SetSpellID4", DbcFieldType::UInt32 },
    { "SetSpellID5", DbcFieldType::UInt32 },
    { "SetSpellID6", DbcFieldType::UInt32 },
    { "SetSpellID7", DbcFieldType::UInt32 },
    { "SetSpellID8", DbcFieldType::UInt32 },
    { "SetThreshold1", DbcFieldType::UInt32 },
    { "SetThreshold2", DbcFieldType::UInt32 },
    { "SetThreshold3", DbcFieldType::UInt32 },
    { "SetThreshold4", DbcFieldType::UInt32 },
    { "SetThreshold5", DbcFieldType::UInt32 },
    { "SetThreshold6", DbcFieldType::UInt32 },
    { "SetThreshold7", DbcFieldType::UInt32 },
    { "SetThreshold8", DbcFieldType::UInt32 },
    { "RequiredSkill", DbcFieldType::UInt32 },
    { "RequiredSkillRank", DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_set = {
    "ItemSet",
    item_set_fields,
    sizeof(item_set_fields) / sizeof(item_set_fields[0])
};

#endif
