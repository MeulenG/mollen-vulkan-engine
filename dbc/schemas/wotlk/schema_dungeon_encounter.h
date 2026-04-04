#ifndef SCHEMA_DUNGEON_ENCOUNTER_H
#define SCHEMA_DUNGEON_ENCOUNTER_H

#include "dbc_schema.h"

static const DbcFieldDef dungeon_encounter_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "MapID",         DbcFieldType::UInt32 },
    { "Difficulty",    DbcFieldType::UInt32 },
    { "OrderIndex",    DbcFieldType::UInt32 },
    { "Bit",           DbcFieldType::UInt32 },
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
    { "SpellIconID",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_dungeon_encounter = {
    "DungeonEncounter",
    dungeon_encounter_fields,
    sizeof(dungeon_encounter_fields) / sizeof(dungeon_encounter_fields[0])
};

#endif
