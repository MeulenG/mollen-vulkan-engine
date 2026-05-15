#ifndef SCHEMA_PET_PERSONALITY_H
#define SCHEMA_PET_PERSONALITY_H

#include "dbc_schema.h"

static const DbcFieldDef pet_personality_fields[] = {
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
    { "HappinessThreshold1", DbcFieldType::UInt32 },
    { "HappinessThreshold2", DbcFieldType::UInt32 },
    { "HappinessThreshold3", DbcFieldType::UInt32 },
    { "HappinessDamage1", DbcFieldType::Float },
    { "HappinessDamage2", DbcFieldType::Float },
    { "HappinessDamage3", DbcFieldType::Float },
};

static const DbcSchema schema_pet_personality = {
    "PetPersonality",
    pet_personality_fields,
    sizeof(pet_personality_fields) / sizeof(pet_personality_fields[0])
};

#endif
