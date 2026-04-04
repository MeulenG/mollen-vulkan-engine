#ifndef SCHEMA_ACHIEVEMENT_CRITERIA_H
#define SCHEMA_ACHIEVEMENT_CRITERIA_H

#include "dbc_schema.h"

static const DbcFieldDef achievement_criteria_fields[] = {
    { "Id",                 DbcFieldType::UInt32 },
    { "AchievementID",      DbcFieldType::UInt32 },
    { "Type",               DbcFieldType::UInt32 },
    { "AssetID",            DbcFieldType::UInt32 },
    { "Quantity",           DbcFieldType::UInt32 },
    { "StartEvent",         DbcFieldType::UInt32 },
    { "StartAsset",         DbcFieldType::UInt32 },
    { "FailEvent",          DbcFieldType::UInt32 },
    { "FailAsset",          DbcFieldType::UInt32 },
    { "Description_enUS",   DbcFieldType::String },
    { "Description_koKR",   DbcFieldType::String },
    { "Description_frFR",   DbcFieldType::String },
    { "Description_deDE",   DbcFieldType::String },
    { "Description_enCN",   DbcFieldType::String },
    { "Description_zhTW",   DbcFieldType::String },
    { "Description_esES",   DbcFieldType::String },
    { "Description_esMX",   DbcFieldType::String },
    { "Description_ruRU",   DbcFieldType::String },
    { "Description_jaJP",   DbcFieldType::String },
    { "Description_ptPT",   DbcFieldType::String },
    { "Description_itIT",   DbcFieldType::String },
    { "Description_Unk1",   DbcFieldType::String },
    { "Description_Unk2",   DbcFieldType::String },
    { "Description_Unk3",   DbcFieldType::String },
    { "Description_Unk4",   DbcFieldType::String },
    { "DescriptionFlags",   DbcFieldType::UInt32 },
    { "Flags",              DbcFieldType::UInt32 },
    { "TimerStartEvent",    DbcFieldType::UInt32 },
    { "TimerAssetID",       DbcFieldType::UInt32 },
    { "TimerTime",          DbcFieldType::UInt32 },
    { "SortOrder",          DbcFieldType::UInt32 },
};

static const DbcSchema schema_achievement_criteria = {
    "Achievement_Criteria",
    achievement_criteria_fields,
    sizeof(achievement_criteria_fields) / sizeof(achievement_criteria_fields[0])
};

#endif
