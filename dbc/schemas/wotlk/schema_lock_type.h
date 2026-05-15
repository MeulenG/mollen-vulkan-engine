#ifndef SCHEMA_LOCK_TYPE_H
#define SCHEMA_LOCK_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef lock_type_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "ResourceName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName" },
    { "ResourceNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Verb_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "Verb_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb" },
    { "VerbFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "CursorName",      DbcFieldType::String },
};

static const DbcSchema schema_lock_type = {
    "LockType",
    lock_type_fields,
    sizeof(lock_type_fields) / sizeof(lock_type_fields[0])
};

#endif
