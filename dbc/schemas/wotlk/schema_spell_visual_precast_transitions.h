#ifndef SCHEMA_SPELL_VISUAL_PRECAST_TRANSITIONS_H
#define SCHEMA_SPELL_VISUAL_PRECAST_TRANSITIONS_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_precast_transitions_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "LoadAnimID", DbcFieldType::String, DbcSemantic::ForeignKey, "loadanim" },
    { "HoldAnimID", DbcFieldType::String, DbcSemantic::ForeignKey, "holdanim" },
};

static const DbcSchema schema_spell_visual_precast_transitions = {
    "SpellVisualPrecastTransitions",
    spell_visual_precast_transitions_fields,
    sizeof(spell_visual_precast_transitions_fields) / sizeof(spell_visual_precast_transitions_fields[0])
};

#endif
