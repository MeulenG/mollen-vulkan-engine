#ifndef ENUM_SPELL_SCHOOL_H
#define ENUM_SPELL_SCHOOL_H

#include "dbc_enum.h"

// Magic damage school index for Spell.dbc. The school MASK (a bitmask
// version of these) is a separate bitmask table — see enum_spell_school_mask.h
// when that lands.
static const DbcEnumValue spell_school_values[] = {
    { 0, "Physical" },
    { 1, "Holy"     },
    { 2, "Fire"     },
    { 3, "Nature"   },
    { 4, "Frost"    },
    { 5, "Shadow"   },
    { 6, "Arcane"   },
};

static const DbcEnum spell_school_enum = {
    "SpellSchool", spell_school_values,
    sizeof(spell_school_values) / sizeof(spell_school_values[0]) };

#endif
