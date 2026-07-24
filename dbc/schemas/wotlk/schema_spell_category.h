#ifndef SCHEMA_SPELL_CATEGORY_H
#define SCHEMA_SPELL_CATEGORY_H

#include "dbc_schema.h"

static const DbcFieldDef spell_category_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_spell_category = {
    "SpellCategory",
    spell_category_fields,
    sizeof(spell_category_fields) / sizeof(spell_category_fields[0])
};

#endif
