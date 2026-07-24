#ifndef SCHEMA_BATTLEMASTER_LIST_H
#define SCHEMA_BATTLEMASTER_LIST_H

#include "dbc_schema.h"

static const DbcFieldDef battlemaster_list_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "MapID1",          DbcFieldType::Int32  },
    { "MapID2",          DbcFieldType::Int32  },
    { "MapID3",          DbcFieldType::Int32  },
    { "MapID4",          DbcFieldType::Int32  },
    { "MapID5",          DbcFieldType::Int32  },
    { "MapID6",          DbcFieldType::Int32  },
    { "MapID7",          DbcFieldType::Int32  },
    { "MapID8",          DbcFieldType::Int32  },
    { "InstanceType",    DbcFieldType::UInt32 },
    { "GroupsAllowed",   DbcFieldType::UInt32 },
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
    { "MaxGroupSize",    DbcFieldType::UInt32 },
    { "HolidayWorldState", DbcFieldType::UInt32 },
    { "MinLevel",        DbcFieldType::UInt32 },
    { "MaxLevel",        DbcFieldType::UInt32 },
};

static const DbcSchema schema_battlemaster_list = {
    "BattlemasterList",
    battlemaster_list_fields,
    sizeof(battlemaster_list_fields) / sizeof(battlemaster_list_fields[0])
};

#endif
