#ifndef ENUM_ITEM_CLASS_H
#define ENUM_ITEM_CLASS_H

#include "dbc_enum.h"

// Top-level item category as seen in Item.dbc and ItemSubClass.dbc.
// WotLK 3.3.5a values; later expansions extended this.
static const DbcEnumValue item_class_values[] = {
    {  0, "Consumable"   },
    {  1, "Container"    },
    {  2, "Weapon"       },
    {  3, "Gem"          },
    {  4, "Armor"        },
    {  5, "Reagent"      },
    {  6, "Projectile"   },
    {  7, "Trade Goods"  },
    {  8, "Generic"      },
    {  9, "Recipe"       },
    { 10, "Money"        },
    { 11, "Quiver"       },
    { 12, "Quest"        },
    { 13, "Key"          },
    { 14, "Permanent"    },
    { 15, "Miscellaneous"},
    { 16, "Glyph"        },
};

static const DbcEnum item_class_enum = {
    "ItemClass", item_class_values,
    sizeof(item_class_values) / sizeof(item_class_values[0]) };

#endif
