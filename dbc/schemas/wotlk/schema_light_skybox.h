#ifndef SCHEMA_LIGHT_SKYBOX_H
#define SCHEMA_LIGHT_SKYBOX_H

#include "dbc_schema.h"

static const DbcFieldDef light_skybox_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
};

static const DbcSchema schema_light_skybox = {
    "LightSkybox",
    light_skybox_fields,
    sizeof(light_skybox_fields) / sizeof(light_skybox_fields[0])
};

#endif
