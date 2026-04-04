#ifndef SCHEMA_CREATURE_SOUND_DATA_H
#define SCHEMA_CREATURE_SOUND_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_sound_data_fields[] = {
    { "Id",                     DbcFieldType::UInt32 },
    { "SoundExertionID",        DbcFieldType::UInt32 },
    { "SoundExertionCriticalID", DbcFieldType::UInt32 },
    { "SoundInjuryID",          DbcFieldType::UInt32 },
    { "SoundInjuryCriticalID",  DbcFieldType::UInt32 },
    { "SoundInjuryCrushingBlowID", DbcFieldType::UInt32 },
    { "SoundDeathID",           DbcFieldType::UInt32 },
    { "SoundStunID",            DbcFieldType::UInt32 },
    { "SoundStandID",           DbcFieldType::UInt32 },
    { "SoundFootstepID",        DbcFieldType::UInt32 },
    { "SoundAggroID",           DbcFieldType::UInt32 },
    { "SoundWingFlapID",        DbcFieldType::UInt32 },
    { "SoundWingGlideID",       DbcFieldType::UInt32 },
    { "SoundAlertID",           DbcFieldType::UInt32 },
    { "SoundFidget1",           DbcFieldType::UInt32 },
    { "SoundFidget2",           DbcFieldType::UInt32 },
    { "SoundFidget3",           DbcFieldType::UInt32 },
    { "SoundFidget4",           DbcFieldType::UInt32 },
    { "SoundFidget5",           DbcFieldType::UInt32 },
    { "SoundCustomAttack1",     DbcFieldType::UInt32 },
    { "SoundCustomAttack2",     DbcFieldType::UInt32 },
    { "SoundCustomAttack3",     DbcFieldType::UInt32 },
    { "SoundCustomAttack4",     DbcFieldType::UInt32 },
    { "NPCSoundID",             DbcFieldType::UInt32 },
    { "LoopSoundID",            DbcFieldType::UInt32 },
    { "CreatureImpactType",     DbcFieldType::UInt32 },
    { "SoundJumpStartID",       DbcFieldType::UInt32 },
    { "SoundJumpEndID",         DbcFieldType::UInt32 },
    { "SoundPetAttackID",       DbcFieldType::UInt32 },
    { "SoundPetOrderID",        DbcFieldType::UInt32 },
    { "SoundPetDismissID",      DbcFieldType::UInt32 },
    { "FidgetDelaySecondsMin",  DbcFieldType::Float  },
    { "FidgetDelaySecondsMax",  DbcFieldType::Float  },
    { "BirthSoundID",           DbcFieldType::UInt32 },
    { "SpellCastDirectedSoundID", DbcFieldType::UInt32 },
    { "SubmergeSoundID",        DbcFieldType::UInt32 },
    { "SubmergedSoundID",       DbcFieldType::UInt32 },
    { "CreatureSoundDataIDPet", DbcFieldType::UInt32 },
};

static const DbcSchema schema_creature_sound_data = {
    "CreatureSoundData",
    creature_sound_data_fields,
    sizeof(creature_sound_data_fields) / sizeof(creature_sound_data_fields[0])
};

#endif
