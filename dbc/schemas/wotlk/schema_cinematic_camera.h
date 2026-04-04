#ifndef SCHEMA_CINEMATIC_CAMERA_H
#define SCHEMA_CINEMATIC_CAMERA_H

#include "dbc_schema.h"

static const DbcFieldDef cinematic_camera_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "Model",       DbcFieldType::String },
    { "SoundID",     DbcFieldType::UInt32 },
    { "OriginX",     DbcFieldType::Float  },
    { "OriginY",     DbcFieldType::Float  },
    { "OriginZ",     DbcFieldType::Float  },
    { "OriginFacing", DbcFieldType::Float },
};

static const DbcSchema schema_cinematic_camera = {
    "CinematicCamera",
    cinematic_camera_fields,
    sizeof(cinematic_camera_fields) / sizeof(cinematic_camera_fields[0])
};

#endif
