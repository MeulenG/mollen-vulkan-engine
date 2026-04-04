#ifndef SCHEMA_GT_OCT_REGEN_MP_H
#define SCHEMA_GT_OCT_REGEN_MP_H

#include "dbc_schema.h"

static const DbcFieldDef gt_oct_regen_mp_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_oct_regen_mp = {
    "gtOCTRegenMP",
    gt_oct_regen_mp_fields,
    sizeof(gt_oct_regen_mp_fields) / sizeof(gt_oct_regen_mp_fields[0])
};

#endif
