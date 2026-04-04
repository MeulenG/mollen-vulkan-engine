#ifndef SCHEMA_GT_COMBAT_RATINGS_H
#define SCHEMA_GT_COMBAT_RATINGS_H

#include "dbc_schema.h"

static const DbcFieldDef gt_combat_ratings_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_combat_ratings = {
    "gtCombatRatings",
    gt_combat_ratings_fields,
    sizeof(gt_combat_ratings_fields) / sizeof(gt_combat_ratings_fields[0])
};

#endif
