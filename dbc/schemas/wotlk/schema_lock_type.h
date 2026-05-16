#ifndef SCHEMA_LOCK_TYPE_H
#define SCHEMA_LOCK_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef lock_type_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
    { "ResourceName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "ResourceName", "Identity" },
    { "ResourceNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "Verb_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "Verb_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Verb", "Identity" },
    { "VerbFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "CursorName", DbcFieldType::String },
};

static const DbcSchema schema_lock_type = {
    "LockType",
    lock_type_fields,
    sizeof(lock_type_fields) / sizeof(lock_type_fields[0])
};

#endif
