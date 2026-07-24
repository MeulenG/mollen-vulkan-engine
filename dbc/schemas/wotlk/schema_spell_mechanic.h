#ifndef SCHEMA_SPELL_MECHANIC_H
#define SCHEMA_SPELL_MECHANIC_H

#include "dbc_schema.h"

static const DbcFieldDef spell_mechanic_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "StateName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName" },
    { "StateNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_spell_mechanic = {
    "SpellMechanic",
    spell_mechanic_fields,
    sizeof(spell_mechanic_fields) / sizeof(spell_mechanic_fields[0])
};

#endif
