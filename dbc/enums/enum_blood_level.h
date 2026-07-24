#ifndef ENUM_BLOOD_LEVEL_H
#define ENUM_BLOOD_LEVEL_H

#include "dbc_enum.h"

// Amount of blood splatter shown when a creature is hit. CreatureDisplayInfo.dbc.
static const DbcEnumValue blood_level_values[] = {
    { 0, "None" },
    { 1, "Low"  },
    { 2, "High" },
};

static const DbcEnum blood_level_enum = {
    "BloodLevel", blood_level_values,
    sizeof(blood_level_values) / sizeof(blood_level_values[0]) };

#endif
