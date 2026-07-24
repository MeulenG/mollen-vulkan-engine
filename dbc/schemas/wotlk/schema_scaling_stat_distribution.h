#ifndef SCHEMA_SCALING_STAT_DISTRIBUTION_H
#define SCHEMA_SCALING_STAT_DISTRIBUTION_H

#include "dbc_schema.h"

static const DbcFieldDef scaling_stat_distribution_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "StatID1", DbcFieldType::Int32 },
    { "StatID2", DbcFieldType::Int32 },
    { "StatID3", DbcFieldType::Int32 },
    { "StatID4", DbcFieldType::Int32 },
    { "StatID5", DbcFieldType::Int32 },
    { "StatID6", DbcFieldType::Int32 },
    { "StatID7", DbcFieldType::Int32 },
    { "StatID8", DbcFieldType::Int32 },
    { "StatID9", DbcFieldType::Int32 },
    { "StatID10", DbcFieldType::Int32 },
    { "Bonus1", DbcFieldType::UInt32 },
    { "Bonus2", DbcFieldType::UInt32 },
    { "Bonus3", DbcFieldType::UInt32 },
    { "Bonus4", DbcFieldType::UInt32 },
    { "Bonus5", DbcFieldType::UInt32 },
    { "Bonus6", DbcFieldType::UInt32 },
    { "Bonus7", DbcFieldType::UInt32 },
    { "Bonus8", DbcFieldType::UInt32 },
    { "Bonus9", DbcFieldType::UInt32 },
    { "Bonus10", DbcFieldType::UInt32 },
    { "MaxLevel", DbcFieldType::UInt32 },
};

static const DbcSchema schema_scaling_stat_distribution = {
    "ScalingStatDistribution",
    scaling_stat_distribution_fields,
    sizeof(scaling_stat_distribution_fields) / sizeof(scaling_stat_distribution_fields[0])
};

#endif
