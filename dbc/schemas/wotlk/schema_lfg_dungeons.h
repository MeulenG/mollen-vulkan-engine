#ifndef SCHEMA_LFG_DUNGEONS_H
#define SCHEMA_LFG_DUNGEONS_H

#include "dbc_schema.h"

static const DbcFieldDef lfg_dungeons_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
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
    { "MinLevel", DbcFieldType::UInt32 },
    { "MaxLevel", DbcFieldType::UInt32 },
    { "TargetLevel", DbcFieldType::UInt32 },
    { "TargetLevelMin", DbcFieldType::UInt32 },
    { "TargetLevelMax", DbcFieldType::UInt32 },
    { "MapID", DbcFieldType::Int32, DbcSemantic::ForeignKey, "map" },
    { "Difficulty", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "TypeID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "type" },
    { "Faction", DbcFieldType::Int32 },
    { "TextureFilename", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "ExpansionLevel", DbcFieldType::UInt32 },
    { "OrderIndex", DbcFieldType::UInt32 },
    { "GroupID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "group" },
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
};

static const DbcSchema schema_lfg_dungeons = {
    "LFGDungeons",
    lfg_dungeons_fields,
    sizeof(lfg_dungeons_fields) / sizeof(lfg_dungeons_fields[0])
};

#endif
