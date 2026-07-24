#ifndef ENUM_CREATURE_TYPE_H
#define ENUM_CREATURE_TYPE_H

#include "dbc_enum.h"

// Mob/NPC family classification. Drives hunter pet eligibility, faction
// behavior, and tracking. CreatureType.dbc and CreatureFamily.dbc.
//
// Note value 13 (Gas Cloud) was added in WotLK for Northrend gathering nodes.
static const DbcEnumValue creature_type_values[] = {
    {  1, "Beast"           },
    {  2, "Dragonkin"       },
    {  3, "Demon"           },
    {  4, "Elemental"       },
    {  5, "Giant"           },
    {  6, "Undead"          },
    {  7, "Humanoid"        },
    {  8, "Critter"         },
    {  9, "Mechanical"      },
    { 10, "Not Specified"   },
    { 11, "Totem"           },
    { 12, "Non-Combat Pet"  },
    { 13, "Gas Cloud"       },
};

static const DbcEnum creature_type_enum = {
    "CreatureType", creature_type_values,
    sizeof(creature_type_values) / sizeof(creature_type_values[0]) };

#endif
