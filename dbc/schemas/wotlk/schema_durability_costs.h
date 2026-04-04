#ifndef SCHEMA_DURABILITY_COSTS_H
#define SCHEMA_DURABILITY_COSTS_H

#include "dbc_schema.h"

static const DbcFieldDef durability_costs_fields[] = {
    { "Id",                 DbcFieldType::UInt32 },
    { "WeaponSubClassCost1", DbcFieldType::UInt32 },
    { "WeaponSubClassCost2", DbcFieldType::UInt32 },
    { "WeaponSubClassCost3", DbcFieldType::UInt32 },
    { "WeaponSubClassCost4", DbcFieldType::UInt32 },
    { "WeaponSubClassCost5", DbcFieldType::UInt32 },
    { "WeaponSubClassCost6", DbcFieldType::UInt32 },
    { "WeaponSubClassCost7", DbcFieldType::UInt32 },
    { "WeaponSubClassCost8", DbcFieldType::UInt32 },
    { "WeaponSubClassCost9", DbcFieldType::UInt32 },
    { "WeaponSubClassCost10", DbcFieldType::UInt32 },
    { "WeaponSubClassCost11", DbcFieldType::UInt32 },
    { "WeaponSubClassCost12", DbcFieldType::UInt32 },
    { "WeaponSubClassCost13", DbcFieldType::UInt32 },
    { "WeaponSubClassCost14", DbcFieldType::UInt32 },
    { "WeaponSubClassCost15", DbcFieldType::UInt32 },
    { "WeaponSubClassCost16", DbcFieldType::UInt32 },
    { "WeaponSubClassCost17", DbcFieldType::UInt32 },
    { "WeaponSubClassCost18", DbcFieldType::UInt32 },
    { "WeaponSubClassCost19", DbcFieldType::UInt32 },
    { "WeaponSubClassCost20", DbcFieldType::UInt32 },
    { "WeaponSubClassCost21", DbcFieldType::UInt32 },
    { "ArmorSubClassCost1",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost2",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost3",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost4",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost5",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost6",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost7",  DbcFieldType::UInt32 },
    { "ArmorSubClassCost8",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_durability_costs = {
    "DurabilityCosts",
    durability_costs_fields,
    sizeof(durability_costs_fields) / sizeof(durability_costs_fields[0])
};

#endif
