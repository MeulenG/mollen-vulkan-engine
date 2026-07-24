#ifndef SCHEMA_VIDEO_HARDWARE_H
#define SCHEMA_VIDEO_HARDWARE_H

#include "dbc_schema.h"

static const DbcFieldDef video_hardware_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "VendorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "vendor" },
    { "DeviceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "device" },
    { "FarclipIdx", DbcFieldType::UInt32 },
    { "TerrainLODDistIdx", DbcFieldType::UInt32 },
    { "TerrainShadowLOD", DbcFieldType::UInt32 },
    { "DetailDoodadDensityIdx", DbcFieldType::UInt32 },
    { "DetailDoodadAlpha", DbcFieldType::UInt32 },
    { "AnimatingDoodadIdx", DbcFieldType::UInt32 },
    { "Trilinear", DbcFieldType::UInt32 },
    { "NumLights", DbcFieldType::UInt32 },
    { "Specularity", DbcFieldType::UInt32 },
    { "WaterLODIdx", DbcFieldType::UInt32 },
    { "ParticleDensityIdx", DbcFieldType::UInt32 },
    { "UnitDrawDistIdx", DbcFieldType::UInt32 },
    { "SmallCullDistIdx", DbcFieldType::UInt32 },
    { "ResolutionIdx", DbcFieldType::UInt32 },
    { "BaseMipLevel", DbcFieldType::UInt32 },
    { "OglOverrides", DbcFieldType::String },
    { "D3dOverrides", DbcFieldType::String },
    { "FixLag", DbcFieldType::UInt32 },
    { "Multisample", DbcFieldType::UInt32 },
    { "Atlasdisable", DbcFieldType::UInt32 },
};

static const DbcSchema schema_video_hardware = {
    "VideoHardware",
    video_hardware_fields,
    sizeof(video_hardware_fields) / sizeof(video_hardware_fields[0])
};

#endif
