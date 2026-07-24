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
    { "PetFoodMask", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "PetTalentType",    DbcFieldType::UInt32 },
    { "CategoryEnumID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "categoryenum" },
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
    { "IconFileID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "iconfile" },
};

static const DbcSchema schema_creature_family = {
    "CreatureFamily",
    creature_family_fields,
    sizeof(creature_family_fields) / sizeof(creature_family_fields[0])
};

#endif
