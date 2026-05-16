#ifndef SCHEMA_CINEMATIC_SEQUENCES_H
#define SCHEMA_CINEMATIC_SEQUENCES_H

#include "dbc_schema.h"

static const DbcFieldDef cinematic_sequences_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
    { "Camera1", DbcFieldType::UInt32 },
    { "Camera2", DbcFieldType::UInt32 },
    { "Camera3", DbcFieldType::UInt32 },
    { "Camera4", DbcFieldType::UInt32 },
    { "Camera5", DbcFieldType::UInt32 },
    { "Camera6", DbcFieldType::UInt32 },
    { "Camera7", DbcFieldType::UInt32 },
    { "Camera8", DbcFieldType::UInt32 },
};

static const DbcSchema schema_cinematic_sequences = {
    "CinematicSequences",
    cinematic_sequences_fields,
    sizeof(cinematic_sequences_fields) / sizeof(cinematic_sequences_fields[0])
};

#endif
