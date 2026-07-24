#ifndef ENUM_POWER_TYPE_H
#define ENUM_POWER_TYPE_H

#include "dbc_enum.h"

// Resource type a class/spell uses. Stored as Int32 because Health is -2.
// ChrClasses.dbc and several Spell.dbc fields.
static const DbcEnumValue power_type_values[] = {
    { -2, "Health"      },
    {  0, "Mana"        },
    {  1, "Rage"        },
    {  2, "Focus"       },
    {  3, "Energy"      },
    {  4, "Happiness"   },
    {  5, "Runic Power" },
    {  6, "Runes"       },
};

static const DbcEnum power_type_enum = {
    "PowerType", power_type_values,
    sizeof(power_type_values) / sizeof(power_type_values[0]) };

#endif
