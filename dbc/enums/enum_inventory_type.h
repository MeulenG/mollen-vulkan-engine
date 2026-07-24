#ifndef ENUM_INVENTORY_TYPE_H
#define ENUM_INVENTORY_TYPE_H

#include "dbc_enum.h"

// Equipment slot category for Item.dbc. Maps roughly to character paper-doll
// slots, with a few special values for non-equipable / virtual slot types.
static const DbcEnumValue inventory_type_values[] = {
    {  0, "Not Equipable"   },
    {  1, "Head"            },
    {  2, "Neck"            },
    {  3, "Shoulder"        },
    {  4, "Body (Shirt)"    },
    {  5, "Chest"           },
    {  6, "Waist"           },
    {  7, "Legs"            },
    {  8, "Feet"            },
    {  9, "Wrists"          },
    { 10, "Hands"           },
    { 11, "Finger"          },
    { 12, "Trinket"         },
    { 13, "One-Hand"        },
    { 14, "Shield"          },
    { 15, "Ranged (Bow)"    },
    { 16, "Cloak"           },
    { 17, "Two-Hand"        },
    { 18, "Bag"             },
    { 19, "Tabard"          },
    { 20, "Robe"            },
    { 21, "Main Hand"       },
    { 22, "Off Hand"        },
    { 23, "Held in Off-Hand"},
    { 24, "Ammo"            },
    { 25, "Thrown"          },
    { 26, "Ranged Right"    },
    { 27, "Quiver"          },
    { 28, "Relic"           },
};

static const DbcEnum inventory_type_enum = {
    "InventoryType", inventory_type_values,
    sizeof(inventory_type_values) / sizeof(inventory_type_values[0]) };

#endif
