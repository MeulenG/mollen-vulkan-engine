#ifndef ENUM_ITEM_QUALITY_H
#define ENUM_ITEM_QUALITY_H

#include "dbc_enum.h"

// Item rarity tier. Drives the colored item name in the client tooltip.
//
//   0 Poor       gray
//   1 Common     white
//   2 Uncommon   green
//   3 Rare       blue
//   4 Epic       purple
//   5 Legendary  orange
//   6 Artifact   light gold
//   7 Heirloom   light gold/teal
static const DbcEnumValue item_quality_values[] = {
    { 0, "Poor"      },
    { 1, "Common"    },
    { 2, "Uncommon"  },
    { 3, "Rare"      },
    { 4, "Epic"      },
    { 5, "Legendary" },
    { 6, "Artifact"  },
    { 7, "Heirloom"  },
};

static const DbcEnum item_quality_enum = {
    "ItemQuality", item_quality_values,
    sizeof(item_quality_values) / sizeof(item_quality_values[0]) };

#endif
