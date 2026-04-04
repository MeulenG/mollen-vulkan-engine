#ifndef SCHEMA_SCALING_STAT_VALUES_H
#define SCHEMA_SCALING_STAT_VALUES_H

#include "dbc_schema.h"

static const DbcFieldDef scaling_stat_values_fields[] = {
    { "Id",                  DbcFieldType::UInt32 },
    { "CharLevel",           DbcFieldType::UInt32 },
    { "ShoulderBudget",      DbcFieldType::UInt32 },
    { "TrinketBudget",       DbcFieldType::UInt32 },
    { "WeaponBudget1H",     DbcFieldType::UInt32 },
    { "RangedBudget",        DbcFieldType::UInt32 },
    { "ClothShoulderArmor",  DbcFieldType::UInt32 },
    { "LeatherShoulderArmor", DbcFieldType::UInt32 },
    { "MailShoulderArmor",   DbcFieldType::UInt32 },
    { "PlateShoulderArmor",  DbcFieldType::UInt32 },
    { "ClothCloakArmor",     DbcFieldType::UInt32 },
    { "ClothChestArmor",     DbcFieldType::UInt32 },
    { "LeatherChestArmor",   DbcFieldType::UInt32 },
    { "MailChestArmor",      DbcFieldType::UInt32 },
    { "PlateChestArmor",     DbcFieldType::UInt32 },
    { "Staff",               DbcFieldType::UInt32 },
    { "WeaponBudget2H",     DbcFieldType::UInt32 },
    { "CasterWeapon1H",     DbcFieldType::UInt32 },
    { "CasterWeapon2H",     DbcFieldType::UInt32 },
    { "CasterOffhand",       DbcFieldType::UInt32 },
    { "Shield",              DbcFieldType::UInt32 },
    { "Wand",                DbcFieldType::UInt32 },
    { "Thrown",              DbcFieldType::UInt32 },
    { "AmmoPouch",           DbcFieldType::UInt32 },
};

static const DbcSchema schema_scaling_stat_values = {
    "ScalingStatValues",
    scaling_stat_values_fields,
    sizeof(scaling_stat_values_fields) / sizeof(scaling_stat_values_fields[0])
};

#endif
