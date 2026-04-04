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
    { "FactionID",         DbcFieldType::UInt32 },
    { "X",                 DbcFieldType::Float  },
    { "Y",                 DbcFieldType::Float  },
    { "Z",                 DbcFieldType::Float  },
    { "ContinentID",       DbcFieldType::UInt32 },
    { "Flags",             DbcFieldType::UInt32 },
    { "AreaID",            DbcFieldType::UInt32 },
    { "Name_enUS",         DbcFieldType::String },
    { "Name_koKR",         DbcFieldType::String },
    { "Name_frFR",         DbcFieldType::String },
    { "Name_deDE",         DbcFieldType::String },
    { "Name_enCN",         DbcFieldType::String },
    { "Name_zhTW",         DbcFieldType::String },
    { "Name_esES",         DbcFieldType::String },
    { "Name_esMX",         DbcFieldType::String },
    { "Name_ruRU",         DbcFieldType::String },
    { "Name_jaJP",         DbcFieldType::String },
    { "Name_ptPT",         DbcFieldType::String },
    { "Name_itIT",         DbcFieldType::String },
    { "Name_Unk1",         DbcFieldType::String },
    { "Name_Unk2",         DbcFieldType::String },
    { "Name_Unk3",         DbcFieldType::String },
    { "Name_Unk4",         DbcFieldType::String },
    { "NameFlags",         DbcFieldType::UInt32 },
    { "Description_enUS",  DbcFieldType::String },
    { "Description_koKR",  DbcFieldType::String },
    { "Description_frFR",  DbcFieldType::String },
    { "Description_deDE",  DbcFieldType::String },
    { "Description_enCN",  DbcFieldType::String },
    { "Description_zhTW",  DbcFieldType::String },
    { "Description_esES",  DbcFieldType::String },
    { "Description_esMX",  DbcFieldType::String },
    { "Description_ruRU",  DbcFieldType::String },
    { "Description_jaJP",  DbcFieldType::String },
    { "Description_ptPT",  DbcFieldType::String },
    { "Description_itIT",  DbcFieldType::String },
    { "Description_Unk1",  DbcFieldType::String },
    { "Description_Unk2",  DbcFieldType::String },
    { "Description_Unk3",  DbcFieldType::String },
    { "Description_Unk4",  DbcFieldType::String },
    { "DescriptionFlags",  DbcFieldType::UInt32 },
    { "WorldStateID",      DbcFieldType::UInt32 },
    { "WorldMapLink",      DbcFieldType::UInt32 },
};

static const DbcSchema schema_area_poi = {
    "AreaPOI",
    area_poi_fields,
    sizeof(area_poi_fields) / sizeof(area_poi_fields[0])
};

#endif
