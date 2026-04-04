#ifndef SCHEMA_ITEM_EXTENDED_COST_H
#define SCHEMA_ITEM_EXTENDED_COST_H

#include "dbc_schema.h"

static const DbcFieldDef item_extended_cost_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "HonorPoints",     DbcFieldType::UInt32 },
    { "ArenaPoints",     DbcFieldType::UInt32 },
    { "ArenaSlotID",     DbcFieldType::UInt32 },
    { "RequiredItem1",   DbcFieldType::UInt32 },
    { "RequiredItem2",   DbcFieldType::UInt32 },
    { "RequiredItem3",   DbcFieldType::UInt32 },
    { "RequiredItem4",   DbcFieldType::UInt32 },
    { "RequiredItem5",   DbcFieldType::UInt32 },
    { "RequiredItemCount1", DbcFieldType::UInt32 },
    { "RequiredItemCount2", DbcFieldType::UInt32 },
    { "RequiredItemCount3", DbcFieldType::UInt32 },
    { "RequiredItemCount4", DbcFieldType::UInt32 },
    { "RequiredItemCount5", DbcFieldType::UInt32 },
    { "RequiredPersonalArenaRating", DbcFieldType::UInt32 },
    { "PurchaseGroupID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_extended_cost = {
    "ItemExtendedCost",
    item_extended_cost_fields,
    sizeof(item_extended_cost_fields) / sizeof(item_extended_cost_fields[0])
};

#endif
