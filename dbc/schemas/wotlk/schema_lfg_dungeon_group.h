#ifndef SCHEMA_LFG_DUNGEON_GROUP_H
#define SCHEMA_LFG_DUNGEON_GROUP_H

#include "dbc_schema.h"

static const DbcFieldDef lfg_dungeon_group_fields[] = {
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
    { "OrderIndex",    DbcFieldType::UInt32 },
    { "ParentGroupID", DbcFieldType::UInt32 },
    { "TypeID",        DbcFieldType::UInt32 },
};

static const DbcSchema schema_lfg_dungeon_group = {
    "LFGDungeonGroup",
    lfg_dungeon_group_fields,
    sizeof(lfg_dungeon_group_fields) / sizeof(lfg_dungeon_group_fields[0])
};

#endif
