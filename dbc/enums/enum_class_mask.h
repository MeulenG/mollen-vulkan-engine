#ifndef ENUM_CLASS_MASK_H
#define ENUM_CLASS_MASK_H

#include "dbc_enum.h"

// Bitmask of playable classes. Spell.dbc and Talent.dbc use this for class
// restrictions. WotLK adds Death Knight at bit 5 (mask 32). Bit 8 (256) and
// bit 9 (512) gaps reflect class slot indices that don't exist.
static const DbcEnumValue class_mask_values[] = {
    {    1, "Warrior"      },
    {    2, "Paladin"      },
    {    4, "Hunter"       },
    {    8, "Rogue"        },
    {   16, "Priest"       },
    {   32, "Death Knight" },
    {   64, "Shaman"       },
    {  128, "Mage"         },
    {  256, "Warlock"      },
    { 1024, "Druid"        },
};

static const DbcEnum class_mask_enum = {
    "ClassMask", class_mask_values,
    sizeof(class_mask_values) / sizeof(class_mask_values[0]) };

#endif
