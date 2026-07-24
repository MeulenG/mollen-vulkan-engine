#ifndef SCHEMA_SPELL_MECHANIC_H
#define SCHEMA_SPELL_MECHANIC_H

#include "dbc_schema.h"

static const DbcFieldDef spell_mechanic_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "StateName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "StateName", "Identity" },
    { "StateNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_spell_mechanic = {
    "SpellMechanic",
    spell_mechanic_fields,
    sizeof(spell_mechanic_fields) / sizeof(spell_mechanic_fields[0])
};

#endif
