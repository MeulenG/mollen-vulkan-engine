#ifndef SCHEMA_CREATURE_DISPLAY_INFO_H
#define SCHEMA_CREATURE_DISPLAY_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef creature_display_info_fields[] = {
    { "Id",                   DbcFieldType::UInt32 },
    { "ModelID",              DbcFieldType::UInt32 },
    { "SoundID",              DbcFieldType::UInt32 },
    { "ExtendedDisplayInfoID", DbcFieldType::UInt32 },
    { "CreatureModelScale",   DbcFieldType::Float  },
    { "CreatureModelAlpha",   DbcFieldType::UInt32 },
    { "TextureVariation1",    DbcFieldType::String },
    { "TextureVariation2",    DbcFieldType::String },
    { "TextureVariation3",    DbcFieldType::String },
    { "PortraitTextureName",  DbcFieldType::String },
    { "BloodLevelID",         DbcFieldType::UInt32 },
    { "BloodID",              DbcFieldType::UInt32 },
    { "NPCSoundID",           DbcFieldType::UInt32 },
    { "ParticleColorID",      DbcFieldType::UInt32 },
    { "CreatureGeosetData",   DbcFieldType::UInt32 },
    { "ObjectEffectPackageID", DbcFieldType::UInt32 },
};

static const DbcSchema schema_creature_display_info = {
    "CreatureDisplayInfo",
    creature_display_info_fields,
    sizeof(creature_display_info_fields) / sizeof(creature_display_info_fields[0])
};

#endif
