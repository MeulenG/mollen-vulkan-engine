#ifndef ENUM_DISPEL_TYPE_H
#define ENUM_DISPEL_TYPE_H

#include "dbc_enum.h"

// Magical effect category for dispel mechanics. Spell.dbc.
static const DbcEnumValue dispel_type_values[] = {
    { 0, "None"         },
    { 1, "Magic"        },
    { 2, "Curse"        },
    { 3, "Disease"      },
    { 4, "Poison"       },
    { 5, "Stealth"      },
    { 6, "Invisibility" },
    { 7, "All"          },
    { 8, "Special"      },
    { 9, "Enrage"       },
};

static const DbcEnum dispel_type_enum = {
    "DispelType", dispel_type_values,
    sizeof(dispel_type_values) / sizeof(dispel_type_values[0]) };

#endif
