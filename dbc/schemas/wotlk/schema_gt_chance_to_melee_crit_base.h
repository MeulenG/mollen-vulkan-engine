#ifndef SCHEMA_GT_CHANCE_TO_MELEE_CRIT_BASE_H
#define SCHEMA_GT_CHANCE_TO_MELEE_CRIT_BASE_H

#include "dbc_schema.h"

static const DbcFieldDef gt_chance_to_melee_crit_base_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_chance_to_melee_crit_base = {
    "gtChanceToMeleeCritBase",
    gt_chance_to_melee_crit_base_fields,
    sizeof(gt_chance_to_melee_crit_base_fields) / sizeof(gt_chance_to_melee_crit_base_fields[0])
};

#endif
