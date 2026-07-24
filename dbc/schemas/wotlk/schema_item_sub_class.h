#ifndef SCHEMA_ITEM_SUB_CLASS_H
#define SCHEMA_ITEM_SUB_CLASS_H

#include "dbc_schema.h"

static const DbcFieldDef item_sub_class_fields[] = {
    { "ClassID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrclasses" },
    { "SubClassID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "subclass" },
    { "PrerequisiteProficiency", DbcFieldType::UInt32 },
    { "PostrequisiteProficiency", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "DisplayFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Visuals" },
    { "WeaponParrySeq", DbcFieldType::UInt32 },
    { "WeaponReadySeq", DbcFieldType::UInt32 },
    { "WeaponAttackSeq", DbcFieldType::UInt32 },
    { "WeaponSwingSize", DbcFieldType::UInt32 },
    { "DisplayName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "DisplayName", "Identity" },
    { "DisplayNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
    { "VerboseName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "VerboseName", "Identity" },
    { "VerboseNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_item_sub_class = {
    "ItemSubClass",
    item_sub_class_fields,
    sizeof(item_sub_class_fields) / sizeof(item_sub_class_fields[0])
};

#endif
