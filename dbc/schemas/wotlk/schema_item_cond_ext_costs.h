#ifndef SCHEMA_ITEM_COND_EXT_COSTS_H
#define SCHEMA_ITEM_COND_EXT_COSTS_H

#include "dbc_schema.h"

static const DbcFieldDef item_cond_ext_costs_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "CondExtendedCost", DbcFieldType::UInt32 },
    { "ItemExtendedCostEntry", DbcFieldType::UInt32 },
    { "ArenaSeason",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_cond_ext_costs = {
    "ItemCondExtCosts",
    item_cond_ext_costs_fields,
    sizeof(item_cond_ext_costs_fields) / sizeof(item_cond_ext_costs_fields[0])
};

#endif
