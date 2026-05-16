#ifndef SCHEMA_SKILL_RACE_CLASS_INFO_H
#define SCHEMA_SKILL_RACE_CLASS_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef skill_race_class_info_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SkillID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "skill" },
    { "RaceMask", DbcFieldType::UInt32, DbcSemantic::Bitmask, "RaceMask" },
    { "ClassMask", DbcFieldType::UInt32, DbcSemantic::Bitmask, "ClassMask" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, "SkillRaceClassInfoFlags", "Classification" },
    { "MinLevel", DbcFieldType::UInt32 },
    { "SkillTierID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "skilltier" },
    { "SkillCostIndex", DbcFieldType::UInt32 },
};

static const DbcSchema schema_skill_race_class_info = {
    "SkillRaceClassInfo",
    skill_race_class_info_fields,
    sizeof(skill_race_class_info_fields) / sizeof(skill_race_class_info_fields[0])
};

#endif
