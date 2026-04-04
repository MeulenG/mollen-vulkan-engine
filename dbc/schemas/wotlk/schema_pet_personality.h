#ifndef SCHEMA_PET_PERSONALITY_H
#define SCHEMA_PET_PERSONALITY_H

#include "dbc_schema.h"

static const DbcFieldDef pet_personality_fields[] = {
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
