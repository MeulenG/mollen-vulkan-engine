#ifndef SCHEMA_ITEM_H
#define SCHEMA_ITEM_H

#include "dbc_schema.h"

static const DbcFieldDef item_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "ClassID",         DbcFieldType::UInt32 },
    { "SubclassID",      DbcFieldType::UInt32 },
    { "SoundOverrideID", DbcFieldType::Int32  },
    { "Material",        DbcFieldType::UInt32 },
    { "DisplayInfoID",   DbcFieldType::UInt32 },
    { "InventoryType",   DbcFieldType::UInt32 },
    { "SheatheType",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_item = {
    "Item",
    item_fields,
    sizeof(item_fields) / sizeof(item_fields[0])
};

#endif // SCHEMA_ITEM_H
