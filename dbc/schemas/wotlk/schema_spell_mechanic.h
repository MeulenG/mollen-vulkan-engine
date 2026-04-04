#ifndef SCHEMA_SPELL_MECHANIC_H
#define SCHEMA_SPELL_MECHANIC_H

#include "dbc_schema.h"

static const DbcFieldDef spell_mechanic_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "StateName_enUS", DbcFieldType::String },
    { "StateName_koKR", DbcFieldType::String },
    { "StateName_frFR", DbcFieldType::String },
    { "StateName_deDE", DbcFieldType::String },
    { "StateName_enCN", DbcFieldType::String },
    { "StateName_zhTW", DbcFieldType::String },
    { "StateName_esES", DbcFieldType::String },
    { "StateName_esMX", DbcFieldType::String },
    { "StateName_ruRU", DbcFieldType::String },
    { "StateName_jaJP", DbcFieldType::String },
    { "StateName_ptPT", DbcFieldType::String },
    { "StateName_itIT", DbcFieldType::String },
    { "StateName_Unk1", DbcFieldType::String },
    { "StateName_Unk2", DbcFieldType::String },
    { "StateName_Unk3", DbcFieldType::String },
    { "StateName_Unk4", DbcFieldType::String },
    { "StateNameFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_mechanic = {
    "SpellMechanic",
    spell_mechanic_fields,
    sizeof(spell_mechanic_fields) / sizeof(spell_mechanic_fields[0])
};

#endif
