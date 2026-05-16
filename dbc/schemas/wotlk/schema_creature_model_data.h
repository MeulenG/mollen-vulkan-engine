#ifndef SCHEMA_CREATURE_MODEL_DATA_H
#define SCHEMA_CREATURE_MODEL_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_model_data_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "ModelName", DbcFieldType::String },
    { "SizeClass", DbcFieldType::UInt32 },
    { "ModelScale", DbcFieldType::Float },
    { "BloodID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "blood" },
    { "FootprintTextureID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "footprinttexture", "Visuals" },
    { "FootprintTextureLength", DbcFieldType::Float, DbcSemantic::Default, nullptr, "Visuals" },
    { "FootprintTextureWidth", DbcFieldType::Float, DbcSemantic::Default, nullptr, "Visuals" },
    { "FootprintParticleScale", DbcFieldType::Float },
    { "FoleyMaterialID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "foleymaterial" },
    { "FootstepShakeSize", DbcFieldType::UInt32 },
    { "DeathThudShakeSize", DbcFieldType::UInt32 },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
    { "CollisionWidth", DbcFieldType::Float },
    { "CollisionHeight", DbcFieldType::Float },
    { "MountHeight", DbcFieldType::Float },
    { "GeoBoxMinX", DbcFieldType::Float },
    { "GeoBoxMinY", DbcFieldType::Float },
    { "GeoBoxMinZ", DbcFieldType::Float },
    { "GeoBoxMaxX", DbcFieldType::Float },
    { "GeoBoxMaxY", DbcFieldType::Float },
    { "GeoBoxMaxZ", DbcFieldType::Float },
    { "WorldEffectScale", DbcFieldType::Float },
    { "AttachedEffectScale", DbcFieldType::Float },
    { "MissileCollisionRadius", DbcFieldType::Float },
    { "MissileCollisionPush", DbcFieldType::Float },
    { "MissileCollisionRaise", DbcFieldType::Float },
};

static const DbcSchema schema_creature_model_data = {
    "CreatureModelData",
    creature_model_data_fields,
    sizeof(creature_model_data_fields) / sizeof(creature_model_data_fields[0])
};

#endif
