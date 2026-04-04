#ifndef SCHEMA_CREATURE_FAMILY_H
#define SCHEMA_CREATURE_FAMILY_H

#include "dbc_schema.h"

static const DbcFieldDef creature_family_fields[] = {
    { "Id",               DbcFieldType::UInt32 },
    { "MinScale",         DbcFieldType::Float  },
    { "MinScaleLevel",    DbcFieldType::UInt32 },
    { "MaxScale",         DbcFieldType::Float  },
    { "MaxScaleLevel",    DbcFieldType::UInt32 },
    { "SkillLine1",       DbcFieldType::UInt32 },
    { "SkillLine2",       DbcFieldType::UInt32 },
    { "PetFoodMask",      DbcFieldType::UInt32 },
    { "PetTalentType",    DbcFieldType::UInt32 },
    { "CategoryEnumID",   DbcFieldType::UInt32 },
    { "Name_enUS",        DbcFieldType::String },
    { "Name_koKR",        DbcFieldType::String },
    { "Name_frFR",        DbcFieldType::String },
    { "Name_deDE",        DbcFieldType::String },
    { "Name_enCN",        DbcFieldType::String },
    { "Name_zhTW",        DbcFieldType::String },
    { "Name_esES",        DbcFieldType::String },
    { "Name_esMX",        DbcFieldType::String },
    { "Name_ruRU",        DbcFieldType::String },
    { "Name_jaJP",        DbcFieldType::String },
    { "Name_ptPT",        DbcFieldType::String },
    { "Name_itIT",        DbcFieldType::String },
    { "Name_Unk1",        DbcFieldType::String },
    { "Name_Unk2",        DbcFieldType::String },
    { "Name_Unk3",        DbcFieldType::String },
    { "Name_Unk4",        DbcFieldType::String },
    { "NameFlags",        DbcFieldType::UInt32 },
    { "IconFileID",       DbcFieldType::UInt32 },
};

static const DbcSchema schema_creature_family = {
    "CreatureFamily",
    creature_family_fields,
    sizeof(creature_family_fields) / sizeof(creature_family_fields[0])
};

#endif
