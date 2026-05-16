#ifndef SCHEMA_SPELL_SHAPESHIFT_FORM_H
#define SCHEMA_SPELL_SHAPESHIFT_FORM_H

#include "dbc_schema.h"

static const DbcFieldDef spell_shapeshift_form_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "BonusActionBar", DbcFieldType::UInt32 },
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
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "CreatureType", DbcFieldType::UInt32, DbcSemantic::Enum, "CreatureType" },
    { "AttackIconID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "attackicon", "Visuals" },
    { "CombatRoundTime", DbcFieldType::UInt32 },
    { "CreatureDisplayID1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CreatureDisplayID2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CreatureDisplayID3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "CreatureDisplayID4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "PresetSpellID1", DbcFieldType::UInt32 },
    { "PresetSpellID2", DbcFieldType::UInt32 },
    { "PresetSpellID3", DbcFieldType::UInt32 },
    { "PresetSpellID4", DbcFieldType::UInt32 },
    { "PresetSpellID5", DbcFieldType::UInt32 },
    { "PresetSpellID6", DbcFieldType::UInt32 },
    { "PresetSpellID7", DbcFieldType::UInt32 },
    { "PresetSpellID8", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_shapeshift_form = {
    "SpellShapeshiftForm",
    spell_shapeshift_form_fields,
    sizeof(spell_shapeshift_form_fields) / sizeof(spell_shapeshift_form_fields[0])
};

#endif
