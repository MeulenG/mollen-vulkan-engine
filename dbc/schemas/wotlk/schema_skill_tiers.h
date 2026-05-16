#ifndef SCHEMA_SKILL_TIERS_H
#define SCHEMA_SKILL_TIERS_H

#include "dbc_schema.h"

static const DbcFieldDef skill_tiers_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Cost1", DbcFieldType::UInt32 },
    { "Cost2", DbcFieldType::UInt32 },
    { "Cost3", DbcFieldType::UInt32 },
    { "Cost4", DbcFieldType::UInt32 },
    { "Cost5", DbcFieldType::UInt32 },
    { "Cost6", DbcFieldType::UInt32 },
    { "Cost7", DbcFieldType::UInt32 },
    { "Cost8", DbcFieldType::UInt32 },
    { "Cost9", DbcFieldType::UInt32 },
    { "Cost10", DbcFieldType::UInt32 },
    { "Cost11", DbcFieldType::UInt32 },
    { "Cost12", DbcFieldType::UInt32 },
    { "Cost13", DbcFieldType::UInt32 },
    { "Cost14", DbcFieldType::UInt32 },
    { "Cost15", DbcFieldType::UInt32 },
    { "Cost16", DbcFieldType::UInt32 },
    { "Value1", DbcFieldType::UInt32 },
    { "Value2", DbcFieldType::UInt32 },
    { "Value3", DbcFieldType::UInt32 },
    { "Value4", DbcFieldType::UInt32 },
    { "Value5", DbcFieldType::UInt32 },
    { "Value6", DbcFieldType::UInt32 },
    { "Value7", DbcFieldType::UInt32 },
    { "Value8", DbcFieldType::UInt32 },
    { "Value9", DbcFieldType::UInt32 },
    { "Value10", DbcFieldType::UInt32 },
    { "Value11", DbcFieldType::UInt32 },
    { "Value12", DbcFieldType::UInt32 },
    { "Value13", DbcFieldType::UInt32 },
    { "Value14", DbcFieldType::UInt32 },
    { "Value15", DbcFieldType::UInt32 },
    { "Value16", DbcFieldType::UInt32 },
};

static const DbcSchema schema_skill_tiers = {
    "SkillTiers",
    skill_tiers_fields,
    sizeof(skill_tiers_fields) / sizeof(skill_tiers_fields[0])
};

#endif
