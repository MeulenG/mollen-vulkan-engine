#ifndef SCHEMA_BANK_BAG_SLOT_PRICES_H
#define SCHEMA_BANK_BAG_SLOT_PRICES_H

#include "dbc_schema.h"

static const DbcFieldDef bank_bag_slot_prices_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Cost", DbcFieldType::UInt32 },
};

static const DbcSchema schema_bank_bag_slot_prices = {
    "BankBagSlotPrices",
    bank_bag_slot_prices_fields,
    sizeof(bank_bag_slot_prices_fields) / sizeof(bank_bag_slot_prices_fields[0])
};

#endif
