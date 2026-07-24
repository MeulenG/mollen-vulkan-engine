#ifndef SCHEMA_SPELL_CHAIN_EFFECTS_H
#define SCHEMA_SPELL_CHAIN_EFFECTS_H

#include "dbc_schema.h"

// Record size: 180 bytes (0xB4) — all 4-byte aligned fields
// Not actually packed, but included for completeness
static const DbcFieldDef spell_chain_effects_fields[] = {
    { "Id",                           DbcFieldType::UInt32 },
    { "AvgSegLen",                    DbcFieldType::Float  },
    { "Width",                        DbcFieldType::Float  },
    { "NoiseScale",                   DbcFieldType::Float  },
    { "TexCoordScale",                DbcFieldType::Float  },
    { "SegDuration",                  DbcFieldType::UInt32 },
    { "SegDelay",                     DbcFieldType::UInt32 },
    { "Texture",                      DbcFieldType::String },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask },
    { "JointCount",                   DbcFieldType::UInt32 },
    { "JointOffsetRadius",            DbcFieldType::Float  },
    { "JointsPerMinorJoint",          DbcFieldType::UInt32 },
    { "MinorJointsPerMajorJoint",     DbcFieldType::UInt32 },
    { "MinorJointScale",              DbcFieldType::Float  },
    { "MajorJointScale",              DbcFieldType::Float  },
    { "JointMoveSpeed",               DbcFieldType::Float  },
    { "JointSmoothness",              DbcFieldType::Float  },
    { "MinDurationBetweenJointJumps", DbcFieldType::Float  },
    { "MaxDurationBetweenJointJumps", DbcFieldType::Float  },
    { "WaveHeight",                   DbcFieldType::Float  },
    { "WaveFreq",                     DbcFieldType::Float  },
    { "WaveSpeed",                    DbcFieldType::Float  },
    { "MinWaveAngle",                 DbcFieldType::Float  },
    { "MaxWaveAngle",                 DbcFieldType::Float  },
    { "MinWaveSpin",                  DbcFieldType::Float  },
    { "MaxWaveSpin",                  DbcFieldType::Float  },
    { "ArcHeight",                    DbcFieldType::Float  },
    { "MinArcAngle",                  DbcFieldType::Float  },
    { "MaxArcAngle",                  DbcFieldType::Float  },
    { "MinArcSpin",                   DbcFieldType::Float  },
    { "MaxArcSpin",                   DbcFieldType::Float  },
    { "DelayBetweenEffects",          DbcFieldType::Float  },
    { "MinFlickerOnDuration",         DbcFieldType::Float  },
    { "MaxFlickerOnDuration",         DbcFieldType::Float  },
    { "MinFlickerOffDuration",        DbcFieldType::Float  },
    { "MaxFlickerOffDuration",        DbcFieldType::Float  },
    { "PulseSpeed",                   DbcFieldType::Float  },
    { "PulseOnLength",                DbcFieldType::Float  },
    { "PulseFadeLength",              DbcFieldType::Float  },
    { "Alpha",                        DbcFieldType::UInt8  },
    { "Red",                          DbcFieldType::UInt8  },
    { "Green",                        DbcFieldType::UInt8  },
    { "Blue",                         DbcFieldType::UInt8  },
    { "BlendMode",                    DbcFieldType::UInt8  },
    { "Combo",                        DbcFieldType::String },
    { "RenderLayer",                  DbcFieldType::UInt32 },
    { "TextureLength",                DbcFieldType::Float  },
};

static const DbcSchema schema_spell_chain_effects = {
    "SpellChainEffects",
    spell_chain_effects_fields,
    sizeof(spell_chain_effects_fields) / sizeof(spell_chain_effects_fields[0]),
    true
};

#endif
