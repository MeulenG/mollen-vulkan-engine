#ifndef SCHEMA_NPC_SOUNDS_H
#define SCHEMA_NPC_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef npc_sounds_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SoundID1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundID2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundID3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundID4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_npc_sounds = {
    "NPCSounds",
    npc_sounds_fields,
    sizeof(npc_sounds_fields) / sizeof(npc_sounds_fields[0])
};

#endif
