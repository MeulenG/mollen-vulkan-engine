#ifndef SCHEMA_CREATURE_SOUND_DATA_H
#define SCHEMA_CREATURE_SOUND_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_sound_data_fields[] = {
    { "Id",                     DbcFieldType::UInt32 },
    { "SoundExertionID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundexertion" },
    { "SoundExertionCriticalID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundexertioncritical" },
    { "SoundInjuryID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundinjury" },
    { "SoundInjuryCriticalID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundinjurycritical" },
    { "SoundInjuryCrushingBlowID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundinjurycrushingblow" },
    { "SoundDeathID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "sounddeath" },
    { "SoundStunID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundstun" },
    { "SoundStandID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundstand" },
    { "SoundFootstepID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundfootstep" },
    { "SoundAggroID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundaggro" },
    { "SoundWingFlapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundwingflap" },
    { "SoundWingGlideID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundwingglide" },
    { "SoundAlertID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundalert" },
    { "SoundFidget1",           DbcFieldType::UInt32 },
    { "SoundFidget2",           DbcFieldType::UInt32 },
    { "SoundFidget3",           DbcFieldType::UInt32 },
    { "SoundFidget4",           DbcFieldType::UInt32 },
    { "SoundFidget5",           DbcFieldType::UInt32 },
    { "SoundCustomAttack1",     DbcFieldType::UInt32 },
    { "SoundCustomAttack2",     DbcFieldType::UInt32 },
    { "SoundCustomAttack3",     DbcFieldType::UInt32 },
    { "SoundCustomAttack4",     DbcFieldType::UInt32 },
    { "NPCSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "npcsounds" },
    { "LoopSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "loopsound" },
    { "CreatureImpactType",     DbcFieldType::UInt32 },
    { "SoundJumpStartID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundjumpstart" },
    { "SoundJumpEndID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundjumpend" },
    { "SoundPetAttackID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundpetattack" },
    { "SoundPetOrderID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundpetorder" },
    { "SoundPetDismissID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundpetdismiss" },
    { "FidgetDelaySecondsMin",  DbcFieldType::Float  },
    { "FidgetDelaySecondsMax",  DbcFieldType::Float  },
    { "BirthSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "birthsound" },
    { "SpellCastDirectedSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellcastdirectedsound" },
    { "SubmergeSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "submergesound" },
    { "SubmergedSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "submergedsound" },
    { "CreatureSoundDataIDPet", DbcFieldType::UInt32 },
};

static const DbcSchema schema_creature_sound_data = {
    "CreatureSoundData",
    creature_sound_data_fields,
    sizeof(creature_sound_data_fields) / sizeof(creature_sound_data_fields[0])
};

#endif
