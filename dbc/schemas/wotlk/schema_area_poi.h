#ifndef SCHEMA_AREA_POI_H
#define SCHEMA_AREA_POI_H

#include "dbc_schema.h"

static const DbcFieldDef area_poi_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Importance", DbcFieldType::UInt32 },
    { "Icon1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon5", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon6", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon7", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon8", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Icon9", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "FactionID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "faction" },
    { "X", DbcFieldType::Float },
    { "Y", DbcFieldType::Float },
    { "Z", DbcFieldType::Float },
    { "ContinentID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "continent" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "AreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "areatable" },
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
    { "WorldStateID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldstate" },
    { "WorldMapLink", DbcFieldType::UInt32 },
};

static const DbcSchema schema_area_poi = {
    "AreaPOI",
    area_poi_fields,
    sizeof(area_poi_fields) / sizeof(area_poi_fields[0])
};

#endif
