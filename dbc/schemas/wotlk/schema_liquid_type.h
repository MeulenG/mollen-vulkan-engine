#ifndef SCHEMA_LIQUID_TYPE_H
#define SCHEMA_LIQUID_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef liquid_type_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "Type", DbcFieldType::UInt32 },
    { "SoundID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "soundentries", "Visuals" },
    { "SpellID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spell" },
    { "MaxDarkenDepth", DbcFieldType::Float },
    { "FogDarkenIntensity", DbcFieldType::Float },
    { "AmbDarkenIntensity", DbcFieldType::Float },
    { "DirDarkenIntensity", DbcFieldType::Float },
    { "LightID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "light" },
    { "ParticleScale", DbcFieldType::Float },
    { "ParticleMovement", DbcFieldType::UInt32 },
    { "ParticleTexSlots", DbcFieldType::UInt32 },
    { "MaterialID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "material" },
    { "Texture1", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture2", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture3", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture4", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture5", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture6", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Color1", DbcFieldType::UInt32, DbcSemantic::Color, nullptr, "Visuals" },
    { "Color2", DbcFieldType::UInt32, DbcSemantic::Color, nullptr, "Visuals" },
    { "Float1", DbcFieldType::Float },
    { "Float2", DbcFieldType::Float },
    { "Float3", DbcFieldType::Float },
    { "Float4", DbcFieldType::Float },
    { "Float5", DbcFieldType::Float },
    { "Float6", DbcFieldType::Float },
    { "Float7", DbcFieldType::Float },
    { "Float8", DbcFieldType::Float },
    { "Float9", DbcFieldType::Float },
    { "Float10", DbcFieldType::Float },
    { "Float11", DbcFieldType::Float },
    { "Float12", DbcFieldType::Float },
    { "Float13", DbcFieldType::Float },
    { "Float14", DbcFieldType::Float },
    { "Float15", DbcFieldType::Float },
    { "Float16", DbcFieldType::Float },
    { "Float17", DbcFieldType::Float },
    { "Float18", DbcFieldType::Float },
    { "Int1", DbcFieldType::UInt32 },
    { "Int2", DbcFieldType::UInt32 },
    { "Int3", DbcFieldType::UInt32 },
    { "Int4", DbcFieldType::UInt32 },
};

static const DbcSchema schema_liquid_type = {
    "LiquidType",
    liquid_type_fields,
    sizeof(liquid_type_fields) / sizeof(liquid_type_fields[0])
};

#endif
