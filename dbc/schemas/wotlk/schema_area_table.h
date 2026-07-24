#ifndef SCHEMA_AREA_TABLE_H
#define SCHEMA_AREA_TABLE_H

#include "dbc_schema.h"

static const DbcFieldDef area_table_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ContinentID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "continent" },
    { "ParentAreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "parentarea" },
    { "AreaBit", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, "AreaFlags", "Classification" },
    { "SoundProviderPref", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundProviderPrefW", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "AmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "ambience" },
    { "ZoneMusic", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "IntroSound", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ExplorationLevel", DbcFieldType::UInt32 },
    { "AreaName_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaName_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "AreaName", "Identity" },
    { "AreaNameFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "FactionGroupMask", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "LiquidTypeID1", DbcFieldType::UInt32 },
    { "LiquidTypeID2", DbcFieldType::UInt32 },
    { "LiquidTypeID3", DbcFieldType::UInt32 },
    { "LiquidTypeID4", DbcFieldType::UInt32 },
    { "MinElevation", DbcFieldType::Float },
    { "AmbientMultiplier", DbcFieldType::Float },
    { "LightID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "light" },
};

static const DbcSchema schema_area_table = {
    "AreaTable",
    area_table_fields,
    sizeof(area_table_fields) / sizeof(area_table_fields[0])
};

#endif // SCHEMA_AREA_TABLE_H
