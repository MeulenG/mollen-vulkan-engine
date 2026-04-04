#ifndef SCHEMA_ITEM_SUB_CLASS_H
#define SCHEMA_ITEM_SUB_CLASS_H

#include "dbc_schema.h"

static const DbcFieldDef item_sub_class_fields[] = {
    { "ClassID",           DbcFieldType::UInt32 },
    { "SubClassID",        DbcFieldType::UInt32 },
    { "PrerequisiteProficiency", DbcFieldType::UInt32 },
    { "PostrequisiteProficiency", DbcFieldType::UInt32 },
    { "Flags",             DbcFieldType::UInt32 },
    { "DisplayFlags",      DbcFieldType::UInt32 },
    { "WeaponParrySeq",    DbcFieldType::UInt32 },
    { "WeaponReadySeq",    DbcFieldType::UInt32 },
    { "WeaponAttackSeq",   DbcFieldType::UInt32 },
    { "WeaponSwingSize",   DbcFieldType::UInt32 },
    { "DisplayName_enUS",  DbcFieldType::String },
    { "DisplayName_koKR",  DbcFieldType::String },
    { "DisplayName_frFR",  DbcFieldType::String },
    { "DisplayName_deDE",  DbcFieldType::String },
    { "DisplayName_enCN",  DbcFieldType::String },
    { "DisplayName_zhTW",  DbcFieldType::String },
    { "DisplayName_esES",  DbcFieldType::String },
    { "DisplayName_esMX",  DbcFieldType::String },
    { "DisplayName_ruRU",  DbcFieldType::String },
    { "DisplayName_jaJP",  DbcFieldType::String },
    { "DisplayName_ptPT",  DbcFieldType::String },
    { "DisplayName_itIT",  DbcFieldType::String },
    { "DisplayName_Unk1",  DbcFieldType::String },
    { "DisplayName_Unk2",  DbcFieldType::String },
    { "DisplayName_Unk3",  DbcFieldType::String },
    { "DisplayName_Unk4",  DbcFieldType::String },
    { "DisplayNameFlags",  DbcFieldType::UInt32 },
    { "VerboseName_enUS",  DbcFieldType::String },
    { "VerboseName_koKR",  DbcFieldType::String },
    { "VerboseName_frFR",  DbcFieldType::String },
    { "VerboseName_deDE",  DbcFieldType::String },
    { "VerboseName_enCN",  DbcFieldType::String },
    { "VerboseName_zhTW",  DbcFieldType::String },
    { "VerboseName_esES",  DbcFieldType::String },
    { "VerboseName_esMX",  DbcFieldType::String },
    { "VerboseName_ruRU",  DbcFieldType::String },
    { "VerboseName_jaJP",  DbcFieldType::String },
    { "VerboseName_ptPT",  DbcFieldType::String },
    { "VerboseName_itIT",  DbcFieldType::String },
    { "VerboseName_Unk1",  DbcFieldType::String },
    { "VerboseName_Unk2",  DbcFieldType::String },
    { "VerboseName_Unk3",  DbcFieldType::String },
    { "VerboseName_Unk4",  DbcFieldType::String },
    { "VerboseNameFlags",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_sub_class = {
    "ItemSubClass",
    item_sub_class_fields,
    sizeof(item_sub_class_fields) / sizeof(item_sub_class_fields[0])
};

#endif
