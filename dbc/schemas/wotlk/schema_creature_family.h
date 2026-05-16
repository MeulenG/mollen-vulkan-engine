#ifndef SCHEMA_CREATURE_FAMILY_H
#define SCHEMA_CREATURE_FAMILY_H

#include "dbc_schema.h"

static const DbcFieldDef creature_family_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "MinScale", DbcFieldType::Float },
    { "MinScaleLevel", DbcFieldType::UInt32 },
    { "MaxScale", DbcFieldType::Float },
    { "MaxScaleLevel", DbcFieldType::UInt32 },
    { "SkillLine1", DbcFieldType::UInt32 },
    { "SkillLine2", DbcFieldType::UInt32 },
    { "PetFoodMask", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "PetTalentType", DbcFieldType::UInt32 },
    { "CategoryEnumID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "categoryenum" },
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
    { "IconFileID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "iconfile", "Visuals" },
};

static const DbcSchema schema_creature_family = {
    "CreatureFamily",
    creature_family_fields,
    sizeof(creature_family_fields) / sizeof(creature_family_fields[0])
};

#endif
