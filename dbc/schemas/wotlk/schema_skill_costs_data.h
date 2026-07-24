#ifndef SCHEMA_SKILL_COSTS_DATA_H
#define SCHEMA_SKILL_COSTS_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef skill_costs_data_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SkillCostID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "skillcost" },
    { "Cost1", DbcFieldType::UInt32 },
    { "Cost2", DbcFieldType::UInt32 },
    { "Cost3", DbcFieldType::UInt32 },
};

static const DbcSchema schema_skill_costs_data = {
    "SkillCostsData",
    skill_costs_data_fields,
    sizeof(skill_costs_data_fields) / sizeof(skill_costs_data_fields[0])
};

#endif
