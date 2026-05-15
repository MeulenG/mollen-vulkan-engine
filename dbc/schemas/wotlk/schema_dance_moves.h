#ifndef SCHEMA_DANCE_MOVES_H
#define SCHEMA_DANCE_MOVES_H

#include "dbc_schema.h"

static const DbcFieldDef dance_moves_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Type",            DbcFieldType::UInt32 },
    { "Param",           DbcFieldType::UInt32 },
    { "Fallback",        DbcFieldType::UInt32 },
    { "Racemask",        DbcFieldType::UInt32 },
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
    { "LockID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "lock" },
    { "Unknown",         DbcFieldType::UInt32 },
};

static const DbcSchema schema_dance_moves = {
    "DanceMoves",
    dance_moves_fields,
    sizeof(dance_moves_fields) / sizeof(dance_moves_fields[0])
};

#endif
