#ifndef SCHEMA_EMOTES_H
#define SCHEMA_EMOTES_H

#include "dbc_schema.h"

static const DbcFieldDef emotes_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "EmoteSlashCommand", DbcFieldType::String },
    { "AnimID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "anim" },
    { "EmoteFlags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "EmoteSpecProc", DbcFieldType::UInt32 },
    { "EmoteSpecProcParam", DbcFieldType::UInt32 },
    { "EventSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "eventsound", "Visuals" },
};

static const DbcSchema schema_emotes = {
    "Emotes",
    emotes_fields,
    sizeof(emotes_fields) / sizeof(emotes_fields[0])
};

#endif
