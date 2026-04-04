#ifndef SCHEMA_SPELL_SHAPESHIFT_FORM_H
#define SCHEMA_SPELL_SHAPESHIFT_FORM_H

#include "dbc_schema.h"

static const DbcFieldDef spell_shapeshift_form_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "BonusActionBar",  DbcFieldType::UInt32 },
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
    { "Flags",           DbcFieldType::UInt32 },
    { "CreatureType",    DbcFieldType::UInt32 },
    { "AttackIconID",    DbcFieldType::UInt32 },
    { "CombatRoundTime", DbcFieldType::UInt32 },
    { "CreatureDisplayID1", DbcFieldType::UInt32 },
    { "CreatureDisplayID2", DbcFieldType::UInt32 },
    { "CreatureDisplayID3", DbcFieldType::UInt32 },
    { "CreatureDisplayID4", DbcFieldType::UInt32 },
    { "PresetSpellID1",  DbcFieldType::UInt32 },
    { "PresetSpellID2",  DbcFieldType::UInt32 },
    { "PresetSpellID3",  DbcFieldType::UInt32 },
    { "PresetSpellID4",  DbcFieldType::UInt32 },
    { "PresetSpellID5",  DbcFieldType::UInt32 },
    { "PresetSpellID6",  DbcFieldType::UInt32 },
    { "PresetSpellID7",  DbcFieldType::UInt32 },
    { "PresetSpellID8",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_shapeshift_form = {
    "SpellShapeshiftForm",
    spell_shapeshift_form_fields,
    sizeof(spell_shapeshift_form_fields) / sizeof(spell_shapeshift_form_fields[0])
};

#endif
