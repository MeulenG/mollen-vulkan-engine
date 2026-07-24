#ifndef SCHEMA_SHEATHE_SOUND_LOOKUPS_H
#define SCHEMA_SHEATHE_SOUND_LOOKUPS_H

#include "dbc_schema.h"

static const DbcFieldDef sheathe_sound_lookups_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ItemClass", DbcFieldType::UInt32 },
    { "ItemSubclass", DbcFieldType::UInt32 },
    { "ItemEnvTypes", DbcFieldType::UInt32 },
    { "IsShield", DbcFieldType::UInt32 },
    { "SheaceSound", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "DrawSound", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_sheathe_sound_lookups = {
    "SheatheSoundLookups",
    sheathe_sound_lookups_fields,
    sizeof(sheathe_sound_lookups_fields) / sizeof(sheathe_sound_lookups_fields[0])
};

#endif
