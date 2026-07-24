#ifndef SCHEMA_SPELL_VISUAL_KIT_AREA_MODEL_H
#define SCHEMA_SPELL_VISUAL_KIT_AREA_MODEL_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_kit_area_model_fields[] = {
    { "Id",      DbcFieldType::UInt32 },
    { "Name",    DbcFieldType::String },
    { "EnumID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "enum" },
};

static const DbcSchema schema_spell_visual_kit_area_model = {
    "SpellVisualKitAreaModel",
    spell_visual_kit_area_model_fields,
    sizeof(spell_visual_kit_area_model_fields) / sizeof(spell_visual_kit_area_model_fields[0])
};

#endif
