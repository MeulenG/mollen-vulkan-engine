#ifndef SCHEMA_TALENT_TAB_H
#define SCHEMA_TALENT_TAB_H

#include "dbc_schema.h"

static const DbcFieldDef talent_tab_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "Name_enUS",     DbcFieldType::String },
    { "Name_koKR",     DbcFieldType::String },
    { "Name_frFR",     DbcFieldType::String },
    { "Name_deDE",     DbcFieldType::String },
    { "Name_enCN",     DbcFieldType::String },
    { "Name_zhTW",     DbcFieldType::String },
    { "Name_esES",     DbcFieldType::String },
    { "Name_esMX",     DbcFieldType::String },
    { "Name_ruRU",     DbcFieldType::String },
    { "Name_jaJP",     DbcFieldType::String },
    { "Name_ptPT",     DbcFieldType::String },
    { "Name_itIT",     DbcFieldType::String },
    { "Name_Unk1",     DbcFieldType::String },
    { "Name_Unk2",     DbcFieldType::String },
    { "Name_Unk3",     DbcFieldType::String },
    { "Name_Unk4",     DbcFieldType::String },
    { "NameFlags",     DbcFieldType::UInt32 },
    { "SpellIconID",   DbcFieldType::UInt32 },
    { "RaceMask",      DbcFieldType::UInt32 },
    { "ClassMask",     DbcFieldType::UInt32 },
    { "PetTalentMask", DbcFieldType::UInt32 },
    { "OrderIndex",    DbcFieldType::UInt32 },
    { "BackgroundFile", DbcFieldType::String },
};

static const DbcSchema schema_talent_tab = {
    "TalentTab",
    talent_tab_fields,
    sizeof(talent_tab_fields) / sizeof(talent_tab_fields[0])
};

#endif
