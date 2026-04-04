#ifndef SCHEMA_SPELL_ITEM_ENCHANTMENT_H
#define SCHEMA_SPELL_ITEM_ENCHANTMENT_H

#include "dbc_schema.h"

static const DbcFieldDef spell_item_enchantment_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Charges",         DbcFieldType::UInt32 },
    { "Effect1",         DbcFieldType::UInt32 },
    { "Effect2",         DbcFieldType::UInt32 },
    { "Effect3",         DbcFieldType::UInt32 },
    { "EffectPointsMin1", DbcFieldType::UInt32 },
    { "EffectPointsMin2", DbcFieldType::UInt32 },
    { "EffectPointsMin3", DbcFieldType::UInt32 },
    { "EffectPointsMax1", DbcFieldType::UInt32 },
    { "EffectPointsMax2", DbcFieldType::UInt32 },
    { "EffectPointsMax3", DbcFieldType::UInt32 },
    { "EffectArg1",      DbcFieldType::UInt32 },
    { "EffectArg2",      DbcFieldType::UInt32 },
    { "EffectArg3",      DbcFieldType::UInt32 },
    { "Name_enUS",       DbcFieldType::String },
    { "Name_koKR",       DbcFieldType::String },
    { "Name_frFR",       DbcFieldType::String },
    { "Name_deDE",       DbcFieldType::String },
    { "Name_enCN",       DbcFieldType::String },
    { "Name_zhTW",       DbcFieldType::String },
    { "Name_esES",       DbcFieldType::String },
    { "Name_esMX",       DbcFieldType::String },
    { "Name_ruRU",       DbcFieldType::String },
    { "Name_jaJP",       DbcFieldType::String },
    { "Name_ptPT",       DbcFieldType::String },
    { "Name_itIT",       DbcFieldType::String },
    { "Name_Unk1",       DbcFieldType::String },
    { "Name_Unk2",       DbcFieldType::String },
    { "Name_Unk3",       DbcFieldType::String },
    { "Name_Unk4",       DbcFieldType::String },
    { "NameFlags",       DbcFieldType::UInt32 },
    { "ItemVisual",      DbcFieldType::UInt32 },
    { "Flags",           DbcFieldType::UInt32 },
    { "SrcItemID",       DbcFieldType::UInt32 },
    { "ConditionId",     DbcFieldType::UInt32 },
    { "RequiredSkillID", DbcFieldType::UInt32 },
    { "RequiredSkillRank", DbcFieldType::UInt32 },
    { "MinLevel",        DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_item_enchantment = {
    "SpellItemEnchantment",
    spell_item_enchantment_fields,
    sizeof(spell_item_enchantment_fields) / sizeof(spell_item_enchantment_fields[0])
};

#endif
