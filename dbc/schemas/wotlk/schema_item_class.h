#ifndef SCHEMA_ITEM_CLASS_H
#define SCHEMA_ITEM_CLASS_H

#include "dbc_schema.h"

static const DbcFieldDef item_class_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SubClassMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "subclassmap" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "ClassName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "ClassName", "Identity" },
    { "ClassNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_item_class = {
    "ItemClass",
    item_class_fields,
    sizeof(item_class_fields) / sizeof(item_class_fields[0])
};

#endif
