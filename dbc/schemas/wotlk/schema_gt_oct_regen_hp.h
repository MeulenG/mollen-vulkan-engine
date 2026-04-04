#ifndef SCHEMA_GT_OCT_REGEN_HP_H
#define SCHEMA_GT_OCT_REGEN_HP_H

#include "dbc_schema.h"

static const DbcFieldDef gt_oct_regen_hp_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_oct_regen_hp = {
    "gtOCTRegenHP",
    gt_oct_regen_hp_fields,
    sizeof(gt_oct_regen_hp_fields) / sizeof(gt_oct_regen_hp_fields[0])
};

#endif
