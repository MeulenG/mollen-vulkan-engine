#ifndef SCHEMA_PARTICLE_COLOR_H
#define SCHEMA_PARTICLE_COLOR_H

#include "dbc_schema.h"

static const DbcFieldDef particle_color_fields[] = {
    { "Id",       DbcFieldType::UInt32 },
    { "Start1",   DbcFieldType::UInt32 },
    { "Start2",   DbcFieldType::UInt32 },
    { "Start3",   DbcFieldType::UInt32 },
    { "Mid1",     DbcFieldType::UInt32 },
    { "Mid2",     DbcFieldType::UInt32 },
    { "Mid3",     DbcFieldType::UInt32 },
    { "End1",     DbcFieldType::UInt32 },
    { "End2",     DbcFieldType::UInt32 },
    { "End3",     DbcFieldType::UInt32 },
};

static const DbcSchema schema_particle_color = {
    "ParticleColor",
    particle_color_fields,
    sizeof(particle_color_fields) / sizeof(particle_color_fields[0])
};

#endif
