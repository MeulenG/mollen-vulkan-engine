#ifndef SCHEMA_CREATURE_SPELL_DATA_H
#define SCHEMA_CREATURE_SPELL_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_spell_data_fields[] = {
    { "Id",       DbcFieldType::UInt32 },
    { "Spell1",   DbcFieldType::UInt32 },
    { "Spell2",   DbcFieldType::UInt32 },
    { "Spell3",   DbcFieldType::UInt32 },
    { "Spell4",   DbcFieldType::UInt32 },
    { "Cooldown1", DbcFieldType::UInt32 },
    { "Cooldown2", DbcFieldType::UInt32 },
    { "Cooldown3", DbcFieldType::UInt32 },
    { "Cooldown4", DbcFieldType::UInt32 },
};

static const DbcSchema schema_creature_spell_data = {
    "CreatureSpellData",
    creature_spell_data_fields,
    sizeof(creature_spell_data_fields) / sizeof(creature_spell_data_fields[0])
};

#endif
