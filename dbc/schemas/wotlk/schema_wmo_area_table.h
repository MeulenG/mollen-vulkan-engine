#ifndef SCHEMA_WMO_AREA_TABLE_H
#define SCHEMA_WMO_AREA_TABLE_H

#include "dbc_schema.h"

static const DbcFieldDef wmo_area_table_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "WmoID",         DbcFieldType::UInt32 },
    { "NameSetID",     DbcFieldType::UInt32 },
    { "WmoGroupID",    DbcFieldType::Int32  },
    { "SoundProviderPreferences", DbcFieldType::UInt32 },
    { "SoundProviderPreferencesUnderwater", DbcFieldType::UInt32 },
    { "AmbienceID",    DbcFieldType::UInt32 },
    { "ZoneMusicID",   DbcFieldType::UInt32 },
    { "IntroSoundID",  DbcFieldType::UInt32 },
    { "Flags",         DbcFieldType::UInt32 },
    { "AreaTableID",   DbcFieldType::UInt32 },
    { "Name_enUS",     DbcFieldType::String },
    { "Name_koKR",     DbcFieldType::String },
    { "Name_frFR",     DbcFieldType::String },
    { "Name_deDE",     DbcFieldType::String },
    { "Name_enCN",     DbcFieldType::String },
    { "Name_zhTW",     DbcFieldType::String },
    { "Name_esES",     DbcFieldType::String },
    { "Name_esMX",     DbcFieldType::String },
    { "Name_ruRU",     DbcFieldType::String },
    { "Name_jaJP",     DbcFieldType::String },
    { "Name_ptPT",     DbcFieldType::String },
    { "Name_itIT",     DbcFieldType::String },
    { "Name_Unk1",     DbcFieldType::String },
    { "Name_Unk2",     DbcFieldType::String },
    { "Name_Unk3",     DbcFieldType::String },
    { "Name_Unk4",     DbcFieldType::String },
    { "NameFlags",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_wmo_area_table = {
    "WMOAreaTable",
    wmo_area_table_fields,
    sizeof(wmo_area_table_fields) / sizeof(wmo_area_table_fields[0])
};

#endif
