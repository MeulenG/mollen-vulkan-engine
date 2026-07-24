#ifndef ENUM_MATERIAL_H
#define ENUM_MATERIAL_H

#include "dbc_enum.h"

// Item material - drives the audio cue when items are picked up / dropped
// and the equip animation in some cases. Item.dbc and ItemDisplayInfo.dbc.
static const DbcEnumValue material_values[] = {
    { -1, "None"        },
    {  0, "Undefined"   },
    {  1, "Consumables" },
    {  2, "Cloth"       },
    {  3, "Leather"     },
    {  4, "Metal"       },
    {  5, "Liquid"      },
    {  6, "Wood"        },
};

static const DbcEnum material_enum = {
    "Material", material_values,
    sizeof(material_values) / sizeof(material_values[0]) };

#endif
