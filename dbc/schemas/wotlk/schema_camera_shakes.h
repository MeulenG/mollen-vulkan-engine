#ifndef SCHEMA_CAMERA_SHAKES_H
#define SCHEMA_CAMERA_SHAKES_H

#include "dbc_schema.h"

static const DbcFieldDef camera_shakes_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ShakeType", DbcFieldType::UInt32 },
    { "Direction", DbcFieldType::UInt32 },
    { "Amplitude", DbcFieldType::Float },
    { "Frequency", DbcFieldType::Float },
    { "Duration", DbcFieldType::Float },
    { "Phase", DbcFieldType::Float },
    { "Coefficient", DbcFieldType::Float },
};

static const DbcSchema schema_camera_shakes = {
    "CameraShakes",
    camera_shakes_fields,
    sizeof(camera_shakes_fields) / sizeof(camera_shakes_fields[0])
};

#endif
