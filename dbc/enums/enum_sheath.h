#ifndef ENUM_SHEATH_H
#define ENUM_SHEATH_H

#include "dbc_enum.h"

// Where/how an unsheathed weapon is shown on the character model when
// stowed. ItemDisplayInfo and Item.dbc.
static const DbcEnumValue sheath_values[] = {
    { 0, "None"             },
    { 1, "Two-Handed (Back)"},
    { 2, "Staff"            },
    { 3, "Main Hand (Waist)"},
    { 4, "Shield"           },
    { 5, "Enchanted Item"   },
    { 6, "Off Hand (Waist)" },
    { 7, "Polearm"          },
};

static const DbcEnum sheath_enum = {
    "Sheath", sheath_values,
    sizeof(sheath_values) / sizeof(sheath_values[0]) };

#endif
