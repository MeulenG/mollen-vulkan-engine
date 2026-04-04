#ifndef SCHEMA_CFG_CATEGORIES_H
#define SCHEMA_CFG_CATEGORIES_H

#include "dbc_schema.h"

static const DbcFieldDef cfg_categories_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "LocaleMask",    DbcFieldType::UInt32 },
    { "CharsetMask",   DbcFieldType::UInt32 },
    { "Flags",         DbcFieldType::UInt32 },
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
};

static const DbcSchema schema_cfg_categories = {
    "Cfg_Categories",
    cfg_categories_fields,
    sizeof(cfg_categories_fields) / sizeof(cfg_categories_fields[0])
};

#endif
