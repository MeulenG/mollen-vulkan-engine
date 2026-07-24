#ifndef SCHEMA_SPELL_RANGE_H
#define SCHEMA_SPELL_RANGE_H

#include "dbc_schema.h"

static const DbcFieldDef spell_range_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "RangeMin1", DbcFieldType::Float },
    { "RangeMin2", DbcFieldType::Float },
    { "RangeMax1", DbcFieldType::Float },
    { "RangeMax2", DbcFieldType::Float },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "DisplayName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
    { "DisplayNameShort_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShort_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort", "Identity" },
    { "DisplayNameShortFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
};

static const DbcSchema schema_spell_range = {
    "SpellRange",
    spell_range_fields,
    sizeof(spell_range_fields) / sizeof(spell_range_fields[0])
};

#endif
