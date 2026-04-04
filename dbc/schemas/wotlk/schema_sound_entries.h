#ifndef SCHEMA_SOUND_ENTRIES_H
#define SCHEMA_SOUND_ENTRIES_H

#include "dbc_schema.h"

static const DbcFieldDef sound_entries_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "SoundType",       DbcFieldType::UInt32 },
    { "Name",            DbcFieldType::String },
    { "File1",           DbcFieldType::String },
    { "File2",           DbcFieldType::String },
    { "File3",           DbcFieldType::String },
    { "File4",           DbcFieldType::String },
    { "File5",           DbcFieldType::String },
    { "File6",           DbcFieldType::String },
    { "File7",           DbcFieldType::String },
    { "File8",           DbcFieldType::String },
    { "File9",           DbcFieldType::String },
    { "File10",          DbcFieldType::String },
    { "Freq1",           DbcFieldType::UInt32 },
    { "Freq2",           DbcFieldType::UInt32 },
    { "Freq3",           DbcFieldType::UInt32 },
    { "Freq4",           DbcFieldType::UInt32 },
    { "Freq5",           DbcFieldType::UInt32 },
    { "Freq6",           DbcFieldType::UInt32 },
    { "Freq7",           DbcFieldType::UInt32 },
    { "Freq8",           DbcFieldType::UInt32 },
    { "Freq9",           DbcFieldType::UInt32 },
    { "Freq10",          DbcFieldType::UInt32 },
    { "DirectoryBase",   DbcFieldType::String },
    { "VolumeFloat",     DbcFieldType::Float  },
    { "Flags",           DbcFieldType::UInt32 },
    { "MinDistance",     DbcFieldType::Float  },
    { "DistanceCutoff",  DbcFieldType::Float  },
    { "EAXDef",          DbcFieldType::UInt32 },
    { "SoundEntriesAdvancedID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_sound_entries = {
    "SoundEntries",
    sound_entries_fields,
    sizeof(sound_entries_fields) / sizeof(sound_entries_fields[0])
};

#endif
