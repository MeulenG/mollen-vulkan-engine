#ifndef SCHEMA_SPELL_FOCUS_OBJECT_H
#define SCHEMA_SPELL_FOCUS_OBJECT_H

#include "dbc_schema.h"

static const DbcFieldDef spell_focus_object_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Name_enUS", DbcFieldType::String },
    { "Name_koKR", DbcFieldType::String },
    { "Name_frFR", DbcFieldType::String },
    { "Name_deDE", DbcFieldType::String },
    { "Name_enCN", DbcFieldType::String },
    { "Name_zhTW", DbcFieldType::String },
    { "Name_esES", DbcFieldType::String },
    { "Name_esMX", DbcFieldType::String },
    { "Name_ruRU", DbcFieldType::String },
    { "Name_jaJP", DbcFieldType::String },
    { "Name_ptPT", DbcFieldType::String },
    { "Name_itIT", DbcFieldType::String },
    { "Name_Unk1", DbcFieldType::String },
    { "Name_Unk2", DbcFieldType::String },
    { "Name_Unk3", DbcFieldType::String },
    { "Name_Unk4", DbcFieldType::String },
    { "NameFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_focus_object = {
    "SpellFocusObject",
    spell_focus_object_fields,
    sizeof(spell_focus_object_fields) / sizeof(spell_focus_object_fields[0])
};

#endif
