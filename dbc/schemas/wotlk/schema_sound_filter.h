#ifndef SCHEMA_SOUND_FILTER_H
#define SCHEMA_SOUND_FILTER_H

#include "dbc_schema.h"

static const DbcFieldDef sound_filter_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
};

static const DbcSchema schema_sound_filter = {
    "SoundFilter",
    sound_filter_fields,
    sizeof(sound_filter_fields) / sizeof(sound_filter_fields[0])
};

#endif
