#ifndef SCHEMA_ITEM_DISPLAY_INFO_H
#define SCHEMA_ITEM_DISPLAY_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef item_display_info_fields[] = {
    { "Id",                   DbcFieldType::UInt32 },
    { "ModelName1",           DbcFieldType::String },
    { "ModelName2",           DbcFieldType::String },
    { "ModelTexture1",        DbcFieldType::String },
    { "ModelTexture2",        DbcFieldType::String },
    { "InventoryIcon1",       DbcFieldType::String },
    { "InventoryIcon2",       DbcFieldType::String },
    { "GeosetGroup1",         DbcFieldType::UInt32 },
    { "GeosetGroup2",         DbcFieldType::UInt32 },
    { "GeosetGroup3",         DbcFieldType::UInt32 },
    { "Flags",                DbcFieldType::UInt32 },
    { "SpellVisualID",        DbcFieldType::UInt32 },
    { "GroupSoundIndex",      DbcFieldType::UInt32 },
    { "HelmetGeosetVisID1",   DbcFieldType::UInt32 },
    { "HelmetGeosetVisID2",   DbcFieldType::UInt32 },
    { "Texture1",             DbcFieldType::String },
    { "Texture2",             DbcFieldType::String },
    { "Texture3",             DbcFieldType::String },
    { "Texture4",             DbcFieldType::String },
    { "Texture5",             DbcFieldType::String },
    { "Texture6",             DbcFieldType::String },
    { "Texture7",             DbcFieldType::String },
    { "Texture8",             DbcFieldType::String },
    { "ItemVisual",           DbcFieldType::UInt32 },
    { "ParticleColorID",      DbcFieldType::UInt32 },
};

static const DbcSchema schema_item_display_info = {
    "ItemDisplayInfo",
    item_display_info_fields,
    sizeof(item_display_info_fields) / sizeof(item_display_info_fields[0])
};

#endif
