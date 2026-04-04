#ifndef SCHEMA_AREA_TABLE_H
#define SCHEMA_AREA_TABLE_H

#include "dbc_schema.h"

static const DbcFieldDef area_table_fields[] = {
    { "Id",                   DbcFieldType::UInt32 },
    { "ContinentID",          DbcFieldType::UInt32 },
    { "ParentAreaID",         DbcFieldType::UInt32 },
    { "AreaBit",              DbcFieldType::UInt32 },
    { "Flags",                DbcFieldType::UInt32 },
    { "SoundProviderPref",    DbcFieldType::UInt32 },
    { "SoundProviderPrefW",   DbcFieldType::UInt32 },
    { "AmbienceID",           DbcFieldType::UInt32 },
    { "ZoneMusic",            DbcFieldType::UInt32 },
    { "IntroSound",           DbcFieldType::UInt32 },
    { "ExplorationLevel",     DbcFieldType::UInt32 },
    { "AreaName_enUS",        DbcFieldType::String },
    { "AreaName_koKR",        DbcFieldType::String },
    { "AreaName_frFR",        DbcFieldType::String },
    { "AreaName_deDE",        DbcFieldType::String },
    { "AreaName_enCN",        DbcFieldType::String },
    { "AreaName_zhTW",        DbcFieldType::String },
    { "AreaName_esES",        DbcFieldType::String },
    { "AreaName_esMX",        DbcFieldType::String },
    { "AreaName_ruRU",        DbcFieldType::String },
    { "AreaName_jaJP",        DbcFieldType::String },
    { "AreaName_ptPT",        DbcFieldType::String },
    { "AreaName_itIT",        DbcFieldType::String },
    { "AreaName_Unk1",        DbcFieldType::String },
    { "AreaName_Unk2",        DbcFieldType::String },
    { "AreaName_Unk3",        DbcFieldType::String },
    { "AreaName_Unk4",        DbcFieldType::String },
    { "AreaNameFlags",        DbcFieldType::UInt32 },
    { "FactionGroupMask",     DbcFieldType::UInt32 },
    { "LiquidTypeID1",        DbcFieldType::UInt32 },
    { "LiquidTypeID2",        DbcFieldType::UInt32 },
    { "LiquidTypeID3",        DbcFieldType::UInt32 },
    { "LiquidTypeID4",        DbcFieldType::UInt32 },
    { "MinElevation",         DbcFieldType::Float  },
    { "AmbientMultiplier",    DbcFieldType::Float  },
    { "LightID",              DbcFieldType::UInt32 },
};

static const DbcSchema schema_area_table = {
    "AreaTable",
    area_table_fields,
    sizeof(area_table_fields) / sizeof(area_table_fields[0])
};

#endif // SCHEMA_AREA_TABLE_H
