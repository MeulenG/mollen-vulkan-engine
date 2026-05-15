#ifndef SCHEMA_SPELL_RANGE_H
#define SCHEMA_SPELL_RANGE_H

#include "dbc_schema.h"

static const DbcFieldDef spell_range_fields[] = {
    { "Id",               DbcFieldType::UInt32 },
    { "RangeMin1",        DbcFieldType::Float  },
    { "RangeMin2",        DbcFieldType::Float  },
    { "RangeMax1",        DbcFieldType::Float  },
    { "RangeMax2",        DbcFieldType::Float  },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "DisplayName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName" },
    { "DisplayNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "DisplayNameShort_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShort_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayNameShort" },
    { "DisplayNameShortFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_spell_range = {
    "SpellRange",
    spell_range_fields,
    sizeof(spell_range_fields) / sizeof(spell_range_fields[0])
};

#endif
