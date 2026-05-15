#ifndef SCHEMA_LFG_DUNGEON_GROUP_H
#define SCHEMA_LFG_DUNGEON_GROUP_H

#include "dbc_schema.h"

static const DbcFieldDef lfg_dungeon_group_fields[] = {
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
    { "OrderIndex",    DbcFieldType::UInt32 },
    { "ParentGroupID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "parentgroup" },
    { "TypeID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "type" },
};

static const DbcSchema schema_lfg_dungeon_group = {
    "LFGDungeonGroup",
    lfg_dungeon_group_fields,
    sizeof(lfg_dungeon_group_fields) / sizeof(lfg_dungeon_group_fields[0])
};

#endif
