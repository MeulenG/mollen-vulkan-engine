#ifndef SCHEMA_ITEM_VISUAL_EFFECTS_H
#define SCHEMA_ITEM_VISUAL_EFFECTS_H

#include "dbc_schema.h"

static const DbcFieldDef item_visual_effects_fields[] = {
    { "Id",    DbcFieldType::UInt32 },
    { "Model", DbcFieldType::String },
};

static const DbcSchema schema_item_visual_effects = {
    "ItemVisualEffects",
    item_visual_effects_fields,
    sizeof(item_visual_effects_fields) / sizeof(item_visual_effects_fields[0])
};

#endif
