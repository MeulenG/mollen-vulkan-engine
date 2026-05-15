#ifndef SCHEMA_WMO_AREA_TABLE_H
#define SCHEMA_WMO_AREA_TABLE_H

#include "dbc_schema.h"

static const DbcFieldDef wmo_area_table_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "WmoID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "wmo" },
    { "NameSetID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "nameset" },
    { "WmoGroupID", DbcFieldType::Int32, DbcSemantic::ForeignKey, "wmogroup" },
    { "SoundProviderPreferences", DbcFieldType::UInt32 },
    { "SoundProviderPreferencesUnderwater", DbcFieldType::UInt32 },
    { "AmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "ambience" },
    { "ZoneMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zonemusic" },
    { "IntroSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "introsound" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "AreaTableID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "areatable" },
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
};

static const DbcSchema schema_wmo_area_table = {
    "WMOAreaTable",
    wmo_area_table_fields,
    sizeof(wmo_area_table_fields) / sizeof(wmo_area_table_fields[0])
};

#endif
