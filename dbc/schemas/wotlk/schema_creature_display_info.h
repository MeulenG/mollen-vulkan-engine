#ifndef SCHEMA_CREATURE_DISPLAY_INFO_H
#define SCHEMA_CREATURE_DISPLAY_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef creature_display_info_fields[] = {
    { "Id",                   DbcFieldType::UInt32 },
    { "ModelID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "creaturemodeldata" },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries" },
    { "ExtendedDisplayInfoID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "extendeddisplayinfo" },
    { "CreatureModelScale",   DbcFieldType::Float  },
    { "CreatureModelAlpha",   DbcFieldType::UInt32 },
    { "TextureVariation1",    DbcFieldType::String },
    { "TextureVariation2",    DbcFieldType::String },
    { "TextureVariation3",    DbcFieldType::String },
    { "PortraitTextureName",  DbcFieldType::String },
    { "BloodLevelID", DbcFieldType::UInt32, DbcSemantic::Enum, "BloodLevel" },
    { "BloodID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "blood" },
    { "NPCSoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "npcsounds" },
    { "ParticleColorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "particlecolor" },
    { "CreatureGeosetData",   DbcFieldType::UInt32 },
    { "ObjectEffectPackageID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "objecteffectpackage" },
};

static const DbcSchema schema_creature_display_info = {
    "CreatureDisplayInfo",
    creature_display_info_fields,
    sizeof(creature_display_info_fields) / sizeof(creature_display_info_fields[0])
};

#endif
