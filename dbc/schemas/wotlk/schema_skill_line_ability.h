#ifndef SCHEMA_SKILL_LINE_ABILITY_H
#define SCHEMA_SKILL_LINE_ABILITY_H

#include "dbc_schema.h"

static const DbcFieldDef skill_line_ability_fields[] = {
    { "Id",                  DbcFieldType::UInt32 },
    { "SkillLine",           DbcFieldType::UInt32 },
    { "SpellID",             DbcFieldType::UInt32 },
    { "RaceMask",            DbcFieldType::UInt32 },
    { "ClassMask",           DbcFieldType::UInt32 },
    { "ExcludeRace",         DbcFieldType::UInt32 },
    { "ExcludeClass",        DbcFieldType::UInt32 },
    { "MinSkillLineRank",    DbcFieldType::UInt32 },
    { "SupercededBySpell",   DbcFieldType::UInt32 },
    { "AcquireMethod",       DbcFieldType::UInt32 },
    { "TrivialSkillLineRankHigh", DbcFieldType::UInt32 },
    { "TrivialSkillLineRankLow", DbcFieldType::UInt32 },
    { "CharacterPoints1",   DbcFieldType::UInt32 },
    { "CharacterPoints2",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_skill_line_ability = {
    "SkillLineAbility",
    skill_line_ability_fields,
    sizeof(skill_line_ability_fields) / sizeof(skill_line_ability_fields[0])
};

#endif
