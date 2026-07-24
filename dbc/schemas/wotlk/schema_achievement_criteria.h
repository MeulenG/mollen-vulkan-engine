#ifndef SCHEMA_ACHIEVEMENT_CRITERIA_H
#define SCHEMA_ACHIEVEMENT_CRITERIA_H

#include "dbc_schema.h"

static const DbcFieldDef achievement_criteria_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "AchievementID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "achievement" },
    { "Type", DbcFieldType::UInt32 },
    { "AssetID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "asset" },
    { "Quantity", DbcFieldType::UInt32 },
    { "StartEvent", DbcFieldType::UInt32 },
    { "StartAsset", DbcFieldType::UInt32 },
    { "FailEvent", DbcFieldType::UInt32 },
    { "FailAsset", DbcFieldType::UInt32 },
    { "Description_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "Description_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Description", "Identity" },
    { "DescriptionFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, "AchievementCriteriaFlags", "Classification" },
    { "TimerStartEvent", DbcFieldType::UInt32 },
    { "TimerAssetID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "timerasset" },
    { "TimerTime", DbcFieldType::UInt32 },
    { "SortOrder", DbcFieldType::UInt32 },
};

static const DbcSchema schema_achievement_criteria = {
    "Achievement_Criteria",
    achievement_criteria_fields,
    sizeof(achievement_criteria_fields) / sizeof(achievement_criteria_fields[0])
};

#endif
