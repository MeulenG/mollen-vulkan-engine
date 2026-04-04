#ifndef SCHEMA_SPELL_DESCRIPTION_VARIABLES_H
#define SCHEMA_SPELL_DESCRIPTION_VARIABLES_H

#include "dbc_schema.h"

static const DbcFieldDef spell_description_variables_fields[] = {
    { "Id",       DbcFieldType::UInt32 },
    { "Formula",  DbcFieldType::String },
};

static const DbcSchema schema_spell_description_variables = {
    "SpellDescriptionVariables",
    spell_description_variables_fields,
    sizeof(spell_description_variables_fields) / sizeof(spell_description_variables_fields[0])
};

#endif
