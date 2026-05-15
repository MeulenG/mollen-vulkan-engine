#ifndef SCHEMA_AREA_POI_H
#define SCHEMA_AREA_POI_H

#include "dbc_schema.h"

static const DbcFieldDef area_poi_fields[] = {
    { "Id",                DbcFieldType::UInt32 },
    { "Importance",        DbcFieldType::UInt32 },
    { "Icon1",             DbcFieldType::UInt32 },
    { "Icon2",             DbcFieldType::UInt32 },
    { "Icon3",             DbcFieldType::UInt32 },
    { "Icon4",             DbcFieldType::UInt32 },
    { "Icon5",             DbcFieldType::UInt32 },
    { "Icon6",             DbcFieldType::UInt32 },
    { "Icon7",             DbcFieldType::UInt32 },
    { "Icon8",             DbcFieldType::UInt32 },
    { "Icon9",             DbcFieldType::UInt32 },
    { "FactionID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "faction" },
    { "X",                 DbcFieldType::Float  },
    { "Y",                 DbcFieldType::Float  },
    { "Z",                 DbcFieldType::Float  },
    { "ContinentID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "continent" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "AreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "areatable" },
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
    { "Description_enUS", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_koKR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_frFR", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_deDE", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_enCN", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_zhTW", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_esES", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_esMX", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_ruRU", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_jaJP", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_ptPT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_itIT", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk1", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk2", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk3", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "Description_Unk4", DbcFieldType::String, DbcSemantic::LocalizedString, "Description" },
    { "DescriptionFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "WorldStateID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldstate" },
    { "WorldMapLink",      DbcFieldType::UInt32 },
};

static const DbcSchema schema_area_poi = {
    "AreaPOI",
    area_poi_fields,
    sizeof(area_poi_fields) / sizeof(area_poi_fields[0])
};

#endif
