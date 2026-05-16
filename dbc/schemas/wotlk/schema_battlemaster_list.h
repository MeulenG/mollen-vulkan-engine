#ifndef SCHEMA_BATTLEMASTER_LIST_H
#define SCHEMA_BATTLEMASTER_LIST_H

#include "dbc_schema.h"

static const DbcFieldDef battlemaster_list_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "MapID1", DbcFieldType::Int32 },
    { "MapID2", DbcFieldType::Int32 },
    { "MapID3", DbcFieldType::Int32 },
    { "MapID4", DbcFieldType::Int32 },
    { "MapID5", DbcFieldType::Int32 },
    { "MapID6", DbcFieldType::Int32 },
    { "MapID7", DbcFieldType::Int32 },
    { "MapID8", DbcFieldType::Int32 },
    { "InstanceType", DbcFieldType::UInt32 },
    { "GroupsAllowed", DbcFieldType::UInt32 },
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
    { "MaxGroupSize", DbcFieldType::UInt32 },
    { "HolidayWorldState", DbcFieldType::UInt32 },
    { "MinLevel", DbcFieldType::UInt32 },
    { "MaxLevel", DbcFieldType::UInt32 },
};

static const DbcSchema schema_battlemaster_list = {
    "BattlemasterList",
    battlemaster_list_fields,
    sizeof(battlemaster_list_fields) / sizeof(battlemaster_list_fields[0])
};

#endif
