#ifndef SCHEMA_GT_BARBERSHOP_COST_BASE_H
#define SCHEMA_GT_BARBERSHOP_COST_BASE_H

#include "dbc_schema.h"

static const DbcFieldDef gt_barbershop_cost_base_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_barbershop_cost_base = {
    "gtBarberShopCostBase",
    gt_barbershop_cost_base_fields,
    sizeof(gt_barbershop_cost_base_fields) / sizeof(gt_barbershop_cost_base_fields[0])
};

#endif
