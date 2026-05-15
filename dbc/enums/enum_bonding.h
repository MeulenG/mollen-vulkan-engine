#ifndef ENUM_BONDING_H
#define ENUM_BONDING_H

#include "dbc_enum.h"

// Item binding rules. Item.dbc.
static const DbcEnumValue bonding_values[] = {
    { 0, "None"          },
    { 1, "Bind on Pickup"},
    { 2, "Bind on Equip" },
    { 3, "Bind on Use"   },
    { 4, "Quest Item"    },
};

static const DbcEnum bonding_enum = {
    "Bonding", bonding_values,
    sizeof(bonding_values) / sizeof(bonding_values[0]) };

#endif
