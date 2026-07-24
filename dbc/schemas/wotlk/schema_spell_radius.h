#ifndef SCHEMA_SPELL_RADIUS_H
#define SCHEMA_SPELL_RADIUS_H

#include "dbc_schema.h"

static const DbcFieldDef spell_radius_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Radius", DbcFieldType::Float },
    { "RadiusPerLevel", DbcFieldType::Float },
    { "MaxRadius", DbcFieldType::Float },
};

static const DbcSchema schema_spell_radius = {
    "SpellRadius",
    spell_radius_fields,
    sizeof(spell_radius_fields) / sizeof(spell_radius_fields[0])
};

#endif
