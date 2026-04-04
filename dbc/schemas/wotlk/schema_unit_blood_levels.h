#ifndef SCHEMA_UNIT_BLOOD_LEVELS_H
#define SCHEMA_UNIT_BLOOD_LEVELS_H

#include "dbc_schema.h"

static const DbcFieldDef unit_blood_levels_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Violance1", DbcFieldType::UInt32 },
    { "Violance2", DbcFieldType::UInt32 },
    { "Violance3", DbcFieldType::UInt32 },
};

static const DbcSchema schema_unit_blood_levels = {
    "UnitBloodLevels",
    unit_blood_levels_fields,
    sizeof(unit_blood_levels_fields) / sizeof(unit_blood_levels_fields[0])
};

#endif
