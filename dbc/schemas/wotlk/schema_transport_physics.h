#ifndef SCHEMA_TRANSPORT_PHYSICS_H
#define SCHEMA_TRANSPORT_PHYSICS_H

#include "dbc_schema.h"

static const DbcFieldDef transport_physics_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "WaveAmp", DbcFieldType::Float },
    { "WaveTimeScale", DbcFieldType::Float },
    { "RollAmp", DbcFieldType::Float },
    { "RollTimeScale", DbcFieldType::Float },
    { "PitchAmp", DbcFieldType::Float },
    { "PitchTimeScale", DbcFieldType::Float },
    { "MaxBank", DbcFieldType::Float },
    { "MaxBankTurnSpeed", DbcFieldType::Float },
    { "SpeedDampingA", DbcFieldType::Float },
    { "SpeedDampingB", DbcFieldType::Float },
};

static const DbcSchema schema_transport_physics = {
    "TransportPhysics",
    transport_physics_fields,
    sizeof(transport_physics_fields) / sizeof(transport_physics_fields[0])
};

#endif
