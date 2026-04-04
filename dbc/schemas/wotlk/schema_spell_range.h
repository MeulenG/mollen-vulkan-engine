#ifndef SCHEMA_SPELL_RANGE_H
#define SCHEMA_SPELL_RANGE_H

#include "dbc_schema.h"

static const DbcFieldDef spell_range_fields[] = {
    { "Id",               DbcFieldType::UInt32 },
    { "RangeMin1",        DbcFieldType::Float  },
    { "RangeMin2",        DbcFieldType::Float  },
    { "RangeMax1",        DbcFieldType::Float  },
    { "RangeMax2",        DbcFieldType::Float  },
    { "Flags",            DbcFieldType::UInt32 },
    { "DisplayName_enUS", DbcFieldType::String },
    { "DisplayName_koKR", DbcFieldType::String },
    { "DisplayName_frFR", DbcFieldType::String },
    { "DisplayName_deDE", DbcFieldType::String },
    { "DisplayName_enCN", DbcFieldType::String },
    { "DisplayName_zhTW", DbcFieldType::String },
    { "DisplayName_esES", DbcFieldType::String },
    { "DisplayName_esMX", DbcFieldType::String },
    { "DisplayName_ruRU", DbcFieldType::String },
    { "DisplayName_jaJP", DbcFieldType::String },
    { "DisplayName_ptPT", DbcFieldType::String },
    { "DisplayName_itIT", DbcFieldType::String },
    { "DisplayName_Unk1", DbcFieldType::String },
    { "DisplayName_Unk2", DbcFieldType::String },
    { "DisplayName_Unk3", DbcFieldType::String },
    { "DisplayName_Unk4", DbcFieldType::String },
    { "DisplayNameFlags",  DbcFieldType::UInt32 },
    { "DisplayNameShort_enUS", DbcFieldType::String },
    { "DisplayNameShort_koKR", DbcFieldType::String },
    { "DisplayNameShort_frFR", DbcFieldType::String },
    { "DisplayNameShort_deDE", DbcFieldType::String },
    { "DisplayNameShort_enCN", DbcFieldType::String },
    { "DisplayNameShort_zhTW", DbcFieldType::String },
    { "DisplayNameShort_esES", DbcFieldType::String },
    { "DisplayNameShort_esMX", DbcFieldType::String },
    { "DisplayNameShort_ruRU", DbcFieldType::String },
    { "DisplayNameShort_jaJP", DbcFieldType::String },
    { "DisplayNameShort_ptPT", DbcFieldType::String },
    { "DisplayNameShort_itIT", DbcFieldType::String },
    { "DisplayNameShort_Unk1", DbcFieldType::String },
    { "DisplayNameShort_Unk2", DbcFieldType::String },
    { "DisplayNameShort_Unk3", DbcFieldType::String },
    { "DisplayNameShort_Unk4", DbcFieldType::String },
    { "DisplayNameShortFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_range = {
    "SpellRange",
    spell_range_fields,
    sizeof(spell_range_fields) / sizeof(spell_range_fields[0])
};

#endif
