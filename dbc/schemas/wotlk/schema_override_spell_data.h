#ifndef SCHEMA_OVERRIDE_SPELL_DATA_H
#define SCHEMA_OVERRIDE_SPELL_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef override_spell_data_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SpellID1", DbcFieldType::UInt32 },
    { "SpellID2", DbcFieldType::UInt32 },
    { "SpellID3", DbcFieldType::UInt32 },
    { "SpellID4", DbcFieldType::UInt32 },
    { "SpellID5", DbcFieldType::UInt32 },
    { "SpellID6", DbcFieldType::UInt32 },
    { "SpellID7", DbcFieldType::UInt32 },
    { "SpellID8", DbcFieldType::UInt32 },
    { "SpellID9", DbcFieldType::UInt32 },
    { "SpellID10", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_override_spell_data = {
    "OverrideSpellData",
    override_spell_data_fields,
    sizeof(override_spell_data_fields) / sizeof(override_spell_data_fields[0])
};

#endif
