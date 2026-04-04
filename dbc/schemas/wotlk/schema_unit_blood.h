#ifndef SCHEMA_UNIT_BLOOD_H
#define SCHEMA_UNIT_BLOOD_H

#include "dbc_schema.h"

static const DbcFieldDef unit_blood_fields[] = {
    { "Id",                     DbcFieldType::UInt32 },
    { "CombatBloodSpurtFront1", DbcFieldType::UInt32 },
    { "CombatBloodSpurtFront2", DbcFieldType::UInt32 },
    { "CombatBloodSpurtBack1",  DbcFieldType::UInt32 },
    { "CombatBloodSpurtBack2",  DbcFieldType::UInt32 },
    { "GroundBlood1",           DbcFieldType::String },
    { "GroundBlood2",           DbcFieldType::String },
    { "GroundBlood3",           DbcFieldType::String },
    { "GroundBlood4",           DbcFieldType::String },
    { "GroundBlood5",           DbcFieldType::String },
};

static const DbcSchema schema_unit_blood = {
    "UnitBlood",
    unit_blood_fields,
    sizeof(unit_blood_fields) / sizeof(unit_blood_fields[0])
};

#endif
