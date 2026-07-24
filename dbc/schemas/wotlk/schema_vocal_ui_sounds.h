#ifndef SCHEMA_VOCAL_UI_SOUNDS_H
#define SCHEMA_VOCAL_UI_SOUNDS_H

#include "dbc_schema.h"

static const DbcFieldDef vocal_ui_sounds_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "VocalUIEnum",    DbcFieldType::UInt32 },
    { "RaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "chrraces" },
    { "NormalSoundID1", DbcFieldType::UInt32 },
    { "NormalSoundID2", DbcFieldType::UInt32 },
    { "PissedSoundID1", DbcFieldType::UInt32 },
    { "PissedSoundID2", DbcFieldType::UInt32 },
};

static const DbcSchema schema_vocal_ui_sounds = {
    "VocalUISounds",
    vocal_ui_sounds_fields,
    sizeof(vocal_ui_sounds_fields) / sizeof(vocal_ui_sounds_fields[0])
};

#endif
