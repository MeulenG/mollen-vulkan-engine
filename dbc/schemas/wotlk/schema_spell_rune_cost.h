#ifndef SCHEMA_SPELL_RUNE_COST_H
#define SCHEMA_SPELL_RUNE_COST_H

#include "dbc_schema.h"

static const DbcFieldDef spell_rune_cost_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "Blood",      DbcFieldType::UInt32 },
    { "Unholy",     DbcFieldType::UInt32 },
    { "Frost",      DbcFieldType::UInt32 },
    { "RunicPower", DbcFieldType::UInt32 },
};

static const DbcSchema schema_spell_rune_cost = {
    "SpellRuneCost",
    spell_rune_cost_fields,
    sizeof(spell_rune_cost_fields) / sizeof(spell_rune_cost_fields[0])
};

#endif
