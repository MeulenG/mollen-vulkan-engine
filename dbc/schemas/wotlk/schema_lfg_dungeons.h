#ifndef SCHEMA_LFG_DUNGEONS_H
#define SCHEMA_LFG_DUNGEONS_H

#include "dbc_schema.h"

static const DbcFieldDef lfg_dungeons_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Name_enUS",       DbcFieldType::String },
    { "Name_koKR",       DbcFieldType::String },
    { "Name_frFR",       DbcFieldType::String },
    { "Name_deDE",       DbcFieldType::String },
    { "Name_enCN",       DbcFieldType::String },
    { "Name_zhTW",       DbcFieldType::String },
    { "Name_esES",       DbcFieldType::String },
    { "Name_esMX",       DbcFieldType::String },
    { "Name_ruRU",       DbcFieldType::String },
    { "Name_jaJP",       DbcFieldType::String },
    { "Name_ptPT",       DbcFieldType::String },
    { "Name_itIT",       DbcFieldType::String },
    { "Name_Unk1",       DbcFieldType::String },
    { "Name_Unk2",       DbcFieldType::String },
    { "Name_Unk3",       DbcFieldType::String },
    { "Name_Unk4",       DbcFieldType::String },
    { "NameFlags",       DbcFieldType::UInt32 },
    { "MinLevel",        DbcFieldType::UInt32 },
    { "MaxLevel",        DbcFieldType::UInt32 },
    { "TargetLevel",     DbcFieldType::UInt32 },
    { "TargetLevelMin",  DbcFieldType::UInt32 },
    { "TargetLevelMax",  DbcFieldType::UInt32 },
    { "MapID",           DbcFieldType::Int32  },
    { "Difficulty",      DbcFieldType::UInt32 },
    { "Flags",           DbcFieldType::UInt32 },
    { "TypeID",          DbcFieldType::UInt32 },
    { "Faction",         DbcFieldType::Int32  },
    { "TextureFilename", DbcFieldType::String },
    { "ExpansionLevel",  DbcFieldType::UInt32 },
    { "OrderIndex",      DbcFieldType::UInt32 },
    { "GroupID",         DbcFieldType::UInt32 },
    { "Description_enUS", DbcFieldType::String },
    { "Description_koKR", DbcFieldType::String },
    { "Description_frFR", DbcFieldType::String },
    { "Description_deDE", DbcFieldType::String },
    { "Description_enCN", DbcFieldType::String },
    { "Description_zhTW", DbcFieldType::String },
    { "Description_esES", DbcFieldType::String },
    { "Description_esMX", DbcFieldType::String },
    { "Description_ruRU", DbcFieldType::String },
    { "Description_jaJP", DbcFieldType::String },
    { "Description_ptPT", DbcFieldType::String },
    { "Description_itIT", DbcFieldType::String },
    { "Description_Unk1", DbcFieldType::String },
    { "Description_Unk2", DbcFieldType::String },
    { "Description_Unk3", DbcFieldType::String },
    { "Description_Unk4", DbcFieldType::String },
    { "DescriptionFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_lfg_dungeons = {
    "LFGDungeons",
    lfg_dungeons_fields,
    sizeof(lfg_dungeons_fields) / sizeof(lfg_dungeons_fields[0])
};

#endif
