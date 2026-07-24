#ifndef SCHEMA_TALENT_TAB_H
#define SCHEMA_TALENT_TAB_H

#include "dbc_schema.h"

static const DbcFieldDef talent_tab_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "SpellIconID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellicon" },
    { "RaceMask", DbcFieldType::UInt32, DbcSemantic::Bitmask, "RaceMask" },
    { "ClassMask", DbcFieldType::UInt32, DbcSemantic::Bitmask, "ClassMask" },
    { "PetTalentMask", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "OrderIndex",    DbcFieldType::UInt32 },
    { "BackgroundFile", DbcFieldType::String },
};

static const DbcSchema schema_talent_tab = {
    "TalentTab",
    talent_tab_fields,
    sizeof(talent_tab_fields) / sizeof(talent_tab_fields[0])
};

#endif
