#ifndef SCHEMA_TALENT_TAB_H
#define SCHEMA_TALENT_TAB_H

#include "dbc_schema.h"

static const DbcFieldDef talent_tab_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "Name_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Name", "Identity" },
    { "NameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
    { "SpellIconID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellicon", "Visuals" },
    { "RaceMask", DbcFieldType::UInt32, DbcSemantic::Bitmask, "RaceMask" },
    { "ClassMask", DbcFieldType::UInt32, DbcSemantic::Bitmask, "ClassMask" },
    { "PetTalentMask", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "OrderIndex", DbcFieldType::UInt32 },
    { "BackgroundFile", DbcFieldType::String },
};

static const DbcSchema schema_talent_tab = {
    "TalentTab",
    talent_tab_fields,
    sizeof(talent_tab_fields) / sizeof(talent_tab_fields[0])
};

#endif
