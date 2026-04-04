#ifndef SCHEMA_SPELL_VISUAL_H
#define SCHEMA_SPELL_VISUAL_H

#include "dbc_schema.h"

static const DbcFieldDef spell_visual_fields[] = {
    { "Id",                    DbcFieldType::UInt32 },
    { "PrecastKit",            DbcFieldType::UInt32 },
    { "CastKit",               DbcFieldType::UInt32 },
    { "ImpactKit",             DbcFieldType::UInt32 },
    { "StateKit",              DbcFieldType::UInt32 },
    { "StateDoneKit",          DbcFieldType::UInt32 },
    { "ChannelKit",            DbcFieldType::UInt32 },
    { "HasMissile",            DbcFieldType::UInt32 },
    { "MissileModel",          DbcFieldType::Int32  },
    { "MissilePathType",       DbcFieldType::UInt32 },
    { "MissileDestAttachment", DbcFieldType::UInt32 },
    { "MissileSound",          DbcFieldType::UInt32 },
    { "AnimEventSoundID",      DbcFieldType::UInt32 },
    { "Flags",                 DbcFieldType::UInt32 },
    { "CasterImpactKit",       DbcFieldType::UInt32 },
    { "TargetImpactKit",       DbcFieldType::UInt32 },
    { "MissileTargetAttachment", DbcFieldType::Int32 },
    { "MissileFollowGroundHeight", DbcFieldType::UInt32 },
    { "MissileFollowGroundDropSpeed", DbcFieldType::UInt32 },
    { "MissileFollowGroundApproach", DbcFieldType::UInt32 },
    { "MissileFollowGroundFlags", DbcFieldType::UInt32 },
    { "MissileMotionID",       DbcFieldType::UInt32 },
    { "MissileTargetingKit",   DbcFieldType::UInt32 },
    { "InstantAreaKit",        DbcFieldType::UInt32 },
    { "ImpactAreaKit",         DbcFieldType::UInt32 },
    { "PersistentAreaKit",     DbcFieldType::UInt32 },
    { "MissileCastOffsetX",    DbcFieldType::Float  },
    { "MissileCastOffsetY",    DbcFieldType::Float  },
    { "MissileCastOffsetZ",    DbcFieldType::Float  },
    { "MissileImpactOffsetX",  DbcFieldType::Float  },
    { "MissileImpactOffsetY",  DbcFieldType::Float  },
    { "MissileImpactOffsetZ",  DbcFieldType::Float  },
};

static const DbcSchema schema_spell_visual = {
    "SpellVisual",
    spell_visual_fields,
    sizeof(spell_visual_fields) / sizeof(spell_visual_fields[0])
};

#endif
