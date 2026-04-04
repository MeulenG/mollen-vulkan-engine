#ifndef SCHEMA_DANCE_MOVES_H
#define SCHEMA_DANCE_MOVES_H

#include "dbc_schema.h"

static const DbcFieldDef dance_moves_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Type",            DbcFieldType::UInt32 },
    { "Param",           DbcFieldType::UInt32 },
    { "Fallback",        DbcFieldType::UInt32 },
    { "Racemask",        DbcFieldType::UInt32 },
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
    { "LockID",          DbcFieldType::UInt32 },
    { "Unknown",         DbcFieldType::UInt32 },
};

static const DbcSchema schema_dance_moves = {
    "DanceMoves",
    dance_moves_fields,
    sizeof(dance_moves_fields) / sizeof(dance_moves_fields[0])
};

#endif
