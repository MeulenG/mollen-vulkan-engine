#ifndef SCHEMA_CURRENCY_TYPES_H
#define SCHEMA_CURRENCY_TYPES_H

#include "dbc_schema.h"

static const DbcFieldDef currency_types_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "ItemID",        DbcFieldType::UInt32 },
    { "Category",      DbcFieldType::UInt32 },
    { "BitIndex",      DbcFieldType::UInt32 },
};

static const DbcSchema schema_currency_types = {
    "CurrencyTypes",
    currency_types_fields,
    sizeof(currency_types_fields) / sizeof(currency_types_fields[0])
};

#endif
