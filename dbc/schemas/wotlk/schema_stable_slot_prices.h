#ifndef SCHEMA_STABLE_SLOT_PRICES_H
#define SCHEMA_STABLE_SLOT_PRICES_H

#include "dbc_schema.h"

static const DbcFieldDef stable_slot_prices_fields[] = {
    { "Id",    DbcFieldType::UInt32 },
    { "Cost",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_stable_slot_prices = {
    "StableSlotPrices",
    stable_slot_prices_fields,
    sizeof(stable_slot_prices_fields) / sizeof(stable_slot_prices_fields[0])
};

#endif
