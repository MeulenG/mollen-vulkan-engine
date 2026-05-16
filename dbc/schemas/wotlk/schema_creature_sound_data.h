#ifndef SCHEMA_CREATURE_SOUND_DATA_H
#define SCHEMA_CREATURE_SOUND_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_sound_data_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "SoundExertionID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundexertion", "Visuals" },
    { "SoundExertionCriticalID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundexertioncritical", "Visuals" },
    { "SoundInjuryID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundinjury", "Visuals" },
    { "SoundInjuryCriticalID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundinjurycritical", "Visuals" },
    { "SoundInjuryCrushingBlowID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundinjurycrushingblow", "Visuals" },
    { "SoundDeathID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "sounddeath", "Visuals" },
    { "SoundStunID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundstun", "Visuals" },
    { "SoundStandID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundstand", "Visuals" },
    { "SoundFootstepID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundfootstep", "Visuals" },
    { "SoundAggroID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundaggro", "Visuals" },
    { "SoundWingFlapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundwingflap", "Visuals" },
    { "SoundWingGlideID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundwingglide", "Visuals" },
    { "SoundAlertID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundalert", "Visuals" },
    { "SoundFidget1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundFidget2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundFidget3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundFidget4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundFidget5", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundCustomAttack1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundCustomAttack2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundCustomAttack3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "SoundCustomAttack4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "npcsounds", "Visuals" },
    { "LoopSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "loopsound", "Visuals" },
    { "CreatureImpactType", DbcFieldType::UInt32 },
    { "SoundJumpStartID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundjumpstart", "Visuals" },
    { "SoundJumpEndID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundjumpend", "Visuals" },
    { "SoundPetAttackID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundpetattack", "Visuals" },
    { "SoundPetOrderID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundpetorder", "Visuals" },
    { "SoundPetDismissID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundpetdismiss", "Visuals" },
    { "FidgetDelaySecondsMin", DbcFieldType::Float },
    { "FidgetDelaySecondsMax", DbcFieldType::Float },
    { "BirthSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "birthsound", "Visuals" },
    { "SpellCastDirectedSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellcastdirectedsound", "Visuals" },
    { "SubmergeSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "submergesound", "Visuals" },
    { "SubmergedSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "submergedsound", "Visuals" },
    { "CreatureSoundDataIDPet", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
};

static const DbcSchema schema_creature_sound_data = {
    "CreatureSoundData",
    creature_sound_data_fields,
    sizeof(creature_sound_data_fields) / sizeof(creature_sound_data_fields[0])
};

#endif
