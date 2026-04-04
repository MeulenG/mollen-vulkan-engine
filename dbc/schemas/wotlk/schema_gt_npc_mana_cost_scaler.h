#ifndef SCHEMA_GT_NPC_MANA_COST_SCALER_H
#define SCHEMA_GT_NPC_MANA_COST_SCALER_H

#include "dbc_schema.h"

static const DbcFieldDef gt_npc_mana_cost_scaler_fields[] = {
    { "Data",  DbcFieldType::Float },
};

static const DbcSchema schema_gt_npc_mana_cost_scaler = {
    "gtNPCManaCostScaler",
    gt_npc_mana_cost_scaler_fields,
    sizeof(gt_npc_mana_cost_scaler_fields) / sizeof(gt_npc_mana_cost_scaler_fields[0])
};

#endif
