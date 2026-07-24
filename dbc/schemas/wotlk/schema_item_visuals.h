#ifndef SCHEMA_ITEM_VISUALS_H
#define SCHEMA_ITEM_VISUALS_H

#include "dbc_schema.h"

static const DbcFieldDef item_visuals_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Slot1", DbcFieldType::UInt32 },
    { "Slot2", DbcFieldType::UInt32 },
    { "Slot3", DbcFieldType::UInt32 },
    { "Slot4", DbcFieldType::UInt32 },
    { "Slot5", DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_visuals = {
    "ItemVisuals",
    item_visuals_fields,
    sizeof(item_visuals_fields) / sizeof(item_visuals_fields[0])
};

#endif
