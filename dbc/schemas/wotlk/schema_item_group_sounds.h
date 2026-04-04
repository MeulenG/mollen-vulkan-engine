#ifndef SCHEMA_ITEM_GROUP_SOUNDS_H
#define SCHEMA_ITEM_GROUP_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef item_group_sounds_fields[] = {
    { "Id",       DbcFieldType::UInt32 },
    { "Sound1",   DbcFieldType::UInt32 },
    { "Sound2",   DbcFieldType::UInt32 },
    { "Sound3",   DbcFieldType::UInt32 },
    { "Sound4",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_group_sounds = {
    "ItemGroupSounds",
    item_group_sounds_fields,
    sizeof(item_group_sounds_fields) / sizeof(item_group_sounds_fields[0])
};

#endif
