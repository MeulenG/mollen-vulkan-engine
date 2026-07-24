#ifndef SCHEMA_SPELL_ITEM_ENCHANTMENT_H
#define SCHEMA_SPELL_ITEM_ENCHANTMENT_H

#include "dbc_schema.h"

static const DbcFieldDef spell_item_enchantment_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Charges", DbcFieldType::UInt32 },
    { "Effect1", DbcFieldType::UInt32 },
    { "Effect2", DbcFieldType::UInt32 },
    { "Effect3", DbcFieldType::UInt32 },
    { "EffectPointsMin1", DbcFieldType::UInt32 },
    { "EffectPointsMin2", DbcFieldType::UInt32 },
    { "EffectPointsMin3", DbcFieldType::UInt32 },
    { "EffectPointsMax1", DbcFieldType::UInt32 },
    { "EffectPointsMax2", DbcFieldType::UInt32 },
    { "EffectPointsMax3", DbcFieldType::UInt32 },
    { "EffectArg1", DbcFieldType::UInt32 },
    { "EffectArg2", DbcFieldType::UInt32 },
    { "EffectArg3", DbcFieldType::UInt32 },
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
    { "ItemVisual", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "SrcItemID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "srcitem" },
    { "ConditionId", DbcFieldType::UInt32 },
    { "RequiredSkillID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "requiredskill" },
    { "RequiredSkillRank", DbcFieldType::UInt32 },
    { "MinLevel", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_item_enchantment = {
    "SpellItemEnchantment",
    spell_item_enchantment_fields,
    sizeof(spell_item_enchantment_fields) / sizeof(spell_item_enchantment_fields[0])
};

#endif
