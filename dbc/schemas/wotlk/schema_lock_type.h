#ifndef SCHEMA_LOCK_TYPE_H
#define SCHEMA_LOCK_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef lock_type_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
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
    { "ResourceName_enUS", DbcFieldType::String },
    { "ResourceName_koKR", DbcFieldType::String },
    { "ResourceName_frFR", DbcFieldType::String },
    { "ResourceName_deDE", DbcFieldType::String },
    { "ResourceName_enCN", DbcFieldType::String },
    { "ResourceName_zhTW", DbcFieldType::String },
    { "ResourceName_esES", DbcFieldType::String },
    { "ResourceName_esMX", DbcFieldType::String },
    { "ResourceName_ruRU", DbcFieldType::String },
    { "ResourceName_jaJP", DbcFieldType::String },
    { "ResourceName_ptPT", DbcFieldType::String },
    { "ResourceName_itIT", DbcFieldType::String },
    { "ResourceName_Unk1", DbcFieldType::String },
    { "ResourceName_Unk2", DbcFieldType::String },
    { "ResourceName_Unk3", DbcFieldType::String },
    { "ResourceName_Unk4", DbcFieldType::String },
    { "ResourceNameFlags", DbcFieldType::UInt32 },
    { "Verb_enUS",       DbcFieldType::String },
    { "Verb_koKR",       DbcFieldType::String },
    { "Verb_frFR",       DbcFieldType::String },
    { "Verb_deDE",       DbcFieldType::String },
    { "Verb_enCN",       DbcFieldType::String },
    { "Verb_zhTW",       DbcFieldType::String },
    { "Verb_esES",       DbcFieldType::String },
    { "Verb_esMX",       DbcFieldType::String },
    { "Verb_ruRU",       DbcFieldType::String },
    { "Verb_jaJP",       DbcFieldType::String },
    { "Verb_ptPT",       DbcFieldType::String },
    { "Verb_itIT",       DbcFieldType::String },
    { "Verb_Unk1",       DbcFieldType::String },
    { "Verb_Unk2",       DbcFieldType::String },
    { "Verb_Unk3",       DbcFieldType::String },
    { "Verb_Unk4",       DbcFieldType::String },
    { "VerbFlags",       DbcFieldType::UInt32 },
    { "CursorName",      DbcFieldType::String },
};

static const DbcSchema schema_lock_type = {
    "LockType",
    lock_type_fields,
    sizeof(lock_type_fields) / sizeof(lock_type_fields[0])
};

#endif
