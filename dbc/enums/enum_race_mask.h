#ifndef ENUM_RACE_MASK_H
#define ENUM_RACE_MASK_H

#include "dbc_enum.h"

// Bitmask of playable races. Used by Spell.dbc, Talent.dbc, etc. for
// race-restriction logic. WotLK 3.3.5a - bit 8 (256) is unused, bit 9 (512)
// is BloodElf, bit 10 (1024) is Draenei.
static const DbcEnumValue race_mask_values[] = {
    {    1, "Human"      },
    {    2, "Orc"        },
    {    4, "Dwarf"      },
    {    8, "Night Elf"  },
    {   16, "Undead"     },
    {   32, "Tauren"     },
    {   64, "Gnome"      },
    {  128, "Troll"      },
    {  512, "Blood Elf"  },
    { 1024, "Draenei"    },
};

static const DbcEnum race_mask_enum = {
    "RaceMask", race_mask_values,
    sizeof(race_mask_values) / sizeof(race_mask_values[0]) };

#endif
