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
    { "Name_enUS",       DbcFieldType::String },
    { "Name_koKR",       DbcFieldType::String },
    { "Name_frFR",       DbcFieldType::String },
    { "Name_deDE",       DbcFieldType::String },
    { "Name_enCN",       DbcFieldType::String },
    { "Name_zhTW",       DbcFieldType::String },
    { "Name_esES",       DbcFieldType::String },
    { "Name_esMX",       DbcFieldType::String },
    { "Name_ruRU",       DbcFieldType::String },
    { "Name_jaJP",       DbcFieldType::String },
    { "Name_ptPT",       DbcFieldType::String },
    { "Name_itIT",       DbcFieldType::String },
    { "Name_Unk1",       DbcFieldType::String },
    { "Name_Unk2",       DbcFieldType::String },
    { "Name_Unk3",       DbcFieldType::String },
    { "Name_Unk4",       DbcFieldType::String },
    { "NameFlags",       DbcFieldType::UInt32 },
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
