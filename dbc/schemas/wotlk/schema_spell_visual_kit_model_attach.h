#ifndef SCHEMA_SPELL_VISUAL_KIT_MODEL_ATTACH_H
#define SCHEMA_SPELL_VISUAL_KIT_MODEL_ATTACH_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_kit_model_attach_fields[] = {
    { "Id",                  DbcFieldType::UInt32 },
    { "ParentSpellVisualKitID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "parentspellvisualkit" },
    { "SpellVisualEffectNameID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellvisualeffectname" },
    { "AttachmentID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "attachment" },
    { "OffsetX",             DbcFieldType::Float  },
    { "OffsetY",             DbcFieldType::Float  },
    { "OffsetZ",             DbcFieldType::Float  },
    { "Yaw",                 DbcFieldType::Float  },
    { "Pitch",               DbcFieldType::Float  },
    { "Roll",                DbcFieldType::Float  },
};

static const DbcSchema schema_spell_visual_kit_model_attach = {
    "SpellVisualKitModelAttach",
    spell_visual_kit_model_attach_fields,
    sizeof(spell_visual_kit_model_attach_fields) / sizeof(spell_visual_kit_model_attach_fields[0])
};

#endif
