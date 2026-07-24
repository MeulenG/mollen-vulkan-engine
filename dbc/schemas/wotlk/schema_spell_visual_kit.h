#ifndef SCHEMA_SPELL_VISUAL_KIT_H
#define SCHEMA_SPELL_VISUAL_KIT_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_kit_fields[] = {
    { "Id",               DbcFieldType::UInt32 },
    { "StartAnimID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "startanim" },
    { "AnimID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "anim" },
    { "AnimKitID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "animkit" },
    { "HeadEffect",       DbcFieldType::UInt32 },
    { "ChestEffect",      DbcFieldType::UInt32 },
    { "BaseEffect",       DbcFieldType::UInt32 },
    { "LeftHandEffect",   DbcFieldType::UInt32 },
    { "RightHandEffect",  DbcFieldType::UInt32 },
    { "BreathEffect",     DbcFieldType::UInt32 },
    { "LeftWeaponEffect",  DbcFieldType::UInt32 },
    { "RightWeaponEffect", DbcFieldType::UInt32 },
    { "SpecialEffect1",   DbcFieldType::UInt32 },
    { "SpecialEffect2",   DbcFieldType::UInt32 },
    { "SpecialEffect3",   DbcFieldType::UInt32 },
    { "WorldEffect",      DbcFieldType::UInt32 },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries" },
    { "ShakeID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "shake" },
    { "CharProc1",        DbcFieldType::UInt32 },
    { "CharProc2",        DbcFieldType::UInt32 },
    { "CharProc3",        DbcFieldType::UInt32 },
    { "CharProc4",        DbcFieldType::UInt32 },
    { "CharParamZero1",   DbcFieldType::Float  },
    { "CharParamZero2",   DbcFieldType::Float  },
    { "CharParamZero3",   DbcFieldType::Float  },
    { "CharParamZero4",   DbcFieldType::Float  },
    { "CharParamOne1",    DbcFieldType::Float  },
    { "CharParamOne2",    DbcFieldType::Float  },
    { "CharParamOne3",    DbcFieldType::Float  },
    { "CharParamOne4",    DbcFieldType::Float  },
    { "CharParamTwo1",    DbcFieldType::Float  },
    { "CharParamTwo2",    DbcFieldType::Float  },
    { "CharParamTwo3",    DbcFieldType::Float  },
    { "CharParamTwo4",    DbcFieldType::Float  },
    { "CharParamThree1",  DbcFieldType::Float  },
    { "CharParamThree2",  DbcFieldType::Float  },
    { "CharParamThree3",  DbcFieldType::Float  },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
};

static const DbcSchema schema_spell_visual_kit = {
    "SpellVisualKit",
    spell_visual_kit_fields,
    sizeof(spell_visual_kit_fields) / sizeof(spell_visual_kit_fields[0])
};

#endif
