#ifndef ENUM_SEX_H
#define ENUM_SEX_H

#include "dbc_enum.h"

// Character/creature gender. Used in CharHairGeosets, CharSections,
// CharStartOutfit, CreatureDisplayInfo (via gender flag), etc.
//
// Value 2 ("None / Both") appears in some legacy data and template rows
// where gender is irrelevant. Most rows use only 0 and 1.
static const DbcEnumValue sex_values[] = {
    { 0, "Male"   },
    { 1, "Female" },
    { 2, "None"   },
};

static const DbcEnum sex_enum = { "Sex", sex_values,
                                  sizeof(sex_values) / sizeof(sex_values[0]) };

#endif
