#ifndef SCHEMA_SOUND_PROVIDER_PREFERENCES_H
#define SCHEMA_SOUND_PROVIDER_PREFERENCES_H

#include "dbc_schema.h"

static const DbcFieldDef sound_provider_preferences_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Description",     DbcFieldType::String },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "EAXEnvironmentSelection", DbcFieldType::UInt32 },
    { "EAXDecayTime",    DbcFieldType::Float  },
    { "EAX2EnvironmentSize", DbcFieldType::Float },
    { "EAX2EnvironmentDiffusion", DbcFieldType::Float },
    { "EAX2Room",        DbcFieldType::Int32  },
    { "EAX2RoomHF",      DbcFieldType::Int32  },
    { "EAX2DecayHFRatio", DbcFieldType::Float },
    { "EAX2Reflections", DbcFieldType::Int32  },
    { "EAX2ReflectionsDelay", DbcFieldType::Float },
    { "EAX2Reverb",      DbcFieldType::Int32  },
    { "EAX2ReverbDelay", DbcFieldType::Float  },
    { "EAX2RoomRolloff", DbcFieldType::Float  },
    { "EAX2AirAbsorption", DbcFieldType::Float },
    { "EAX3RoomLF",      DbcFieldType::Int32  },
    { "EAX3DecayLFRatio", DbcFieldType::Float },
    { "EAX3EchoTime",   DbcFieldType::Float  },
    { "EAX3EchoDepth",  DbcFieldType::Float  },
    { "EAX3ModulationTime", DbcFieldType::Float },
    { "EAX3ModulationDepth", DbcFieldType::Float },
    { "EAX3HFReference", DbcFieldType::Float },
    { "EAX3LFReference", DbcFieldType::Float },
};

static const DbcSchema schema_sound_provider_preferences = {
    "SoundProviderPreferences",
    sound_provider_preferences_fields,
    sizeof(sound_provider_preferences_fields) / sizeof(sound_provider_preferences_fields[0])
};

#endif
