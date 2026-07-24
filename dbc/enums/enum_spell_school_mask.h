#ifndef ENUM_SPELL_SCHOOL_MASK_H
#define ENUM_SPELL_SCHOOL_MASK_H

#include "dbc_enum.h"

// Bitmask form of SpellSchool. A spell can be multi-school (e.g. Frostfire
// Bolt = Frost | Fire). Spell.dbc.SchoolMask.
static const DbcEnumValue spell_school_mask_values[] = {
    {  1, "Physical" },
    {  2, "Holy"     },
    {  4, "Fire"     },
    {  8, "Nature"   },
    { 16, "Frost"    },
    { 32, "Shadow"   },
    { 64, "Arcane"   },
};

static const DbcEnum spell_school_mask_enum = {
    "SpellSchoolMask", spell_school_mask_values,
    sizeof(spell_school_mask_values) / sizeof(spell_school_mask_values[0]) };

#endif
