#ifndef SCHEMA_SPELL_ICON_H
#define SCHEMA_SPELL_ICON_H

#include "dbc_schema.h"

static const DbcFieldDef spell_icon_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "IconPath", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_spell_icon = {
    "SpellIcon",
    spell_icon_fields,
    sizeof(spell_icon_fields) / sizeof(spell_icon_fields[0])
};

#endif
