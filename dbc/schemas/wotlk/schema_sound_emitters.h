#ifndef SCHEMA_SOUND_EMITTERS_H
#define SCHEMA_SOUND_EMITTERS_H

#include "dbc_schema.h"

static const DbcFieldDef sound_emitters_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "PositionX",     DbcFieldType::Float  },
    { "PositionY",     DbcFieldType::Float  },
    { "PositionZ",     DbcFieldType::Float  },
    { "DirectionX",    DbcFieldType::Float  },
    { "DirectionY",    DbcFieldType::Float  },
    { "DirectionZ",    DbcFieldType::Float  },
    { "SoundEntriesID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries" },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "Name",          DbcFieldType::String },
};

static const DbcSchema schema_sound_emitters = {
    "SoundEmitters",
    sound_emitters_fields,
    sizeof(sound_emitters_fields) / sizeof(sound_emitters_fields[0])
};

#endif
