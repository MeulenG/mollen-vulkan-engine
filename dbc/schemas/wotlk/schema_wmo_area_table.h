#ifndef SCHEMA_WMO_AREA_TABLE_H
#define SCHEMA_WMO_AREA_TABLE_H

#include "dbc_schema.h"

static const DbcFieldDef wmo_area_table_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "WmoID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "wmo" },
    { "NameSetID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "nameset", "Identity" },
    { "WmoGroupID", DbcFieldType::Int32, DbcSemantic::ForeignKey, "wmogroup" },
    { "SoundProviderPreferences", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundProviderPreferencesUnderwater", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "AmbienceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "ambience" },
    { "ZoneMusicID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "zonemusic", "Visuals" },
    { "IntroSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "introsound", "Visuals" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "AreaTableID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "areatable" },
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
};

static const DbcSchema schema_wmo_area_table = {
    "WMOAreaTable",
    wmo_area_table_fields,
    sizeof(wmo_area_table_fields) / sizeof(wmo_area_table_fields[0])
};

#endif
