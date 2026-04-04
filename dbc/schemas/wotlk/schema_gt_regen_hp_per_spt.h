#ifndef SCHEMA_GT_REGEN_HP_PER_SPT_H
#define SCHEMA_GT_REGEN_HP_PER_SPT_H

#include "dbc_schema.h"

static const DbcFieldDef gt_regen_hp_per_spt_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_regen_hp_per_spt = {
    "gtRegenHPPerSpt",
    gt_regen_hp_per_spt_fields,
    sizeof(gt_regen_hp_per_spt_fields) / sizeof(gt_regen_hp_per_spt_fields[0])
};

#endif
