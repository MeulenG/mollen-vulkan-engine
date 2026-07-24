#ifndef SCHEMA_DUNGEON_ENCOUNTER_H
#define SCHEMA_DUNGEON_ENCOUNTER_H

#include "dbc_schema.h"

static const DbcFieldDef dungeon_encounter_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "Difficulty",    DbcFieldType::UInt32 },
    { "OrderIndex",    DbcFieldType::UInt32 },
    { "Bit",           DbcFieldType::UInt32 },
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
    { "SpellIconID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellicon" },
};

static const DbcSchema schema_dungeon_encounter = {
    "DungeonEncounter",
    dungeon_encounter_fields,
    sizeof(dungeon_encounter_fields) / sizeof(dungeon_encounter_fields[0])
};

#endif
