#ifndef SCHEMA_ITEM_CLASS_H
#define SCHEMA_ITEM_CLASS_H

#include "dbc_schema.h"

static const DbcFieldDef item_class_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "SubClassMapID",   DbcFieldType::UInt32 },
    { "Flags",           DbcFieldType::UInt32 },
    { "ClassName_enUS",  DbcFieldType::String },
    { "ClassName_koKR",  DbcFieldType::String },
    { "ClassName_frFR",  DbcFieldType::String },
    { "ClassName_deDE",  DbcFieldType::String },
    { "ClassName_enCN",  DbcFieldType::String },
    { "ClassName_zhTW",  DbcFieldType::String },
    { "ClassName_esES",  DbcFieldType::String },
    { "ClassName_esMX",  DbcFieldType::String },
    { "ClassName_ruRU",  DbcFieldType::String },
    { "ClassName_jaJP",  DbcFieldType::String },
    { "ClassName_ptPT",  DbcFieldType::String },
    { "ClassName_itIT",  DbcFieldType::String },
    { "ClassName_Unk1",  DbcFieldType::String },
    { "ClassName_Unk2",  DbcFieldType::String },
    { "ClassName_Unk3",  DbcFieldType::String },
    { "ClassName_Unk4",  DbcFieldType::String },
    { "ClassNameFlags",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_class = {
    "ItemClass",
    item_class_fields,
    sizeof(item_class_fields) / sizeof(item_class_fields[0])
};

#endif
