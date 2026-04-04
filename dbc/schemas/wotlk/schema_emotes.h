#ifndef SCHEMA_EMOTES_H
#define SCHEMA_EMOTES_H

#include "dbc_schema.h"

static const DbcFieldDef emotes_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "EmoteSlashCommand", DbcFieldType::String },
    { "AnimID",         DbcFieldType::UInt32 },
    { "EmoteFlags",     DbcFieldType::UInt32 },
    { "EmoteSpecProc",  DbcFieldType::UInt32 },
    { "EmoteSpecProcParam", DbcFieldType::UInt32 },
    { "EventSoundID",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_emotes = {
    "Emotes",
    emotes_fields,
    sizeof(emotes_fields) / sizeof(emotes_fields[0])
};

#endif
