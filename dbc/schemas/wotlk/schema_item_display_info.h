#ifndef SCHEMA_ITEM_DISPLAY_INFO_H
#define SCHEMA_ITEM_DISPLAY_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef item_display_info_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ModelName1", DbcFieldType::String },
    { "ModelName2", DbcFieldType::String },
    { "ModelTexture1", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "ModelTexture2", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "InventoryIcon1", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "InventoryIcon2", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "GeosetGroup1", DbcFieldType::UInt32 },
    { "GeosetGroup2", DbcFieldType::UInt32 },
    { "GeosetGroup3", DbcFieldType::UInt32 },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "SpellVisualID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "spellvisual", "Visuals" },
    { "GroupSoundIndex", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "HelmetGeosetVisID1", DbcFieldType::UInt32 },
    { "HelmetGeosetVisID2", DbcFieldType::UInt32 },
    { "Texture1", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture2", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture3", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture4", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture5", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture6", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture7", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "Texture8", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "ItemVisual", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "ParticleColorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "particlecolor", "Visuals" },
};

static const DbcSchema schema_item_display_info = {
    "ItemDisplayInfo",
    item_display_info_fields,
    sizeof(item_display_info_fields) / sizeof(item_display_info_fields[0])
};

#endif
