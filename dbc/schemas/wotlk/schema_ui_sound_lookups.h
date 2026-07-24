#ifndef SCHEMA_UI_SOUND_LOOKUPS_H
#define SCHEMA_UI_SOUND_LOOKUPS_H

#include "dbc_schema.h"

static const DbcFieldDef ui_sound_lookups_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SoundEntriesID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
};

static const DbcSchema schema_ui_sound_lookups = {
    "UISoundLookups",
    ui_sound_lookups_fields,
    sizeof(ui_sound_lookups_fields) / sizeof(ui_sound_lookups_fields[0])
};

#endif
