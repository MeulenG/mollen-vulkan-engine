#ifndef SCHEMA_ITEM_CLASS_H
#define SCHEMA_ITEM_CLASS_H

#include "dbc_schema.h"

static const DbcFieldDef item_class_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "SubClassMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "subclassmap" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "ClassName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName" },
    { "ClassNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_item_class = {
    "ItemClass",
    item_class_fields,
    sizeof(item_class_fields) / sizeof(item_class_fields[0])
};

#endif
