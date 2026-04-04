#ifndef SCHEMA_SPELL_VISUAL_PRECAST_TRANSITIONS_H
#define SCHEMA_SPELL_VISUAL_PRECAST_TRANSITIONS_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_precast_transitions_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "LoadAnimID", DbcFieldType::String },
    { "HoldAnimID", DbcFieldType::String },
};

static const DbcSchema schema_spell_visual_precast_transitions = {
    "SpellVisualPrecastTransitions",
    spell_visual_precast_transitions_fields,
    sizeof(spell_visual_precast_transitions_fields) / sizeof(spell_visual_precast_transitions_fields[0])
};

#endif
