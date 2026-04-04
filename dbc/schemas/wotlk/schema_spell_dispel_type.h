#ifndef SCHEMA_SPELL_DISPEL_TYPE_H
#define SCHEMA_SPELL_DISPEL_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef spell_dispel_type_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "Name_enUS",      DbcFieldType::String },
    { "Name_koKR",      DbcFieldType::String },
    { "Name_frFR",      DbcFieldType::String },
    { "Name_deDE",      DbcFieldType::String },
    { "Name_enCN",      DbcFieldType::String },
    { "Name_zhTW",      DbcFieldType::String },
    { "Name_esES",      DbcFieldType::String },
    { "Name_esMX",      DbcFieldType::String },
    { "Name_ruRU",      DbcFieldType::String },
    { "Name_jaJP",      DbcFieldType::String },
    { "Name_ptPT",      DbcFieldType::String },
    { "Name_itIT",      DbcFieldType::String },
    { "Name_Unk1",      DbcFieldType::String },
    { "Name_Unk2",      DbcFieldType::String },
    { "Name_Unk3",      DbcFieldType::String },
    { "Name_Unk4",      DbcFieldType::String },
    { "NameFlags",      DbcFieldType::UInt32 },
    { "ImmunityFlags",  DbcFieldType::UInt32 },
    { "InternalName",   DbcFieldType::String },
    { "Unknown1",       DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_dispel_type = {
    "SpellDispelType",
    spell_dispel_type_fields,
    sizeof(spell_dispel_type_fields) / sizeof(spell_dispel_type_fields[0])
};

#endif
