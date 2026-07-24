#ifndef SCHEMA_ITEM_H
#define SCHEMA_ITEM_H

#include "dbc_schema.h"

static const DbcFieldDef item_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "ClassID", DbcFieldType::UInt32, DbcSemantic::Enum, "ItemClass" },
    { "SubclassID",      DbcFieldType::UInt32 },
    { "SoundOverrideID", DbcFieldType::Int32, DbcSemantic::ForeignKey, "soundoverride" },
    { "Material", DbcFieldType::UInt32, DbcSemantic::Enum, "Material" },
    { "DisplayInfoID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "itemdisplayinfo" },
    { "InventoryType", DbcFieldType::UInt32, DbcSemantic::Enum, "InventoryType" },
    { "SheatheType", DbcFieldType::UInt32, DbcSemantic::Enum, "Sheath" },
};

static const DbcSchema schema_item = {
    "Item",
    item_fields,
    sizeof(item_fields) / sizeof(item_fields[0])
};

#endif // SCHEMA_ITEM_H
