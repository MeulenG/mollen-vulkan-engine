#ifndef SCHEMA_GT_OCT_CLASS_COMBAT_RATING_SCALAR_H
#define SCHEMA_GT_OCT_CLASS_COMBAT_RATING_SCALAR_H

#include "dbc_schema.h"

static const DbcFieldDef gt_oct_class_combat_rating_scalar_fields[] = {
    { "Data",   DbcFieldType::Float  },
    { "Data2",  DbcFieldType::Float  },
};

static const DbcSchema schema_gt_oct_class_combat_rating_scalar = {
    "gtOCTClassCombatRatingScalar",
    gt_oct_class_combat_rating_scalar_fields,
    sizeof(gt_oct_class_combat_rating_scalar_fields) / sizeof(gt_oct_class_combat_rating_scalar_fields[0])
};

#endif
