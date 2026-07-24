#ifndef SCHEMA_SPELL_VISUAL_EFFECT_NAME_H
#define SCHEMA_SPELL_VISUAL_EFFECT_NAME_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_effect_name_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "FilePath", DbcFieldType::String },
    { "AreaEffectSize", DbcFieldType::Float },
    { "Scale", DbcFieldType::Float },
    { "MinAllowedScale", DbcFieldType::Float },
    { "MaxAllowedScale", DbcFieldType::Float },
};

static const DbcSchema schema_spell_visual_effect_name = {
    "SpellVisualEffectName",
    spell_visual_effect_name_fields,
    sizeof(spell_visual_effect_name_fields) / sizeof(spell_visual_effect_name_fields[0])
};

#endif
