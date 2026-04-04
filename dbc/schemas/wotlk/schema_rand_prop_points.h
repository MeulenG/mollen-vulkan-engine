#ifndef SCHEMA_RAND_PROP_POINTS_H
#define SCHEMA_RAND_PROP_POINTS_H

#include "dbc_schema.h"

static const DbcFieldDef rand_prop_points_fields[] = {
    { "Id",          DbcFieldType::UInt32 },
    { "Epic1",       DbcFieldType::UInt32 },
    { "Epic2",       DbcFieldType::UInt32 },
    { "Epic3",       DbcFieldType::UInt32 },
    { "Epic4",       DbcFieldType::UInt32 },
    { "Epic5",       DbcFieldType::UInt32 },
    { "Rare1",       DbcFieldType::UInt32 },
    { "Rare2",       DbcFieldType::UInt32 },
    { "Rare3",       DbcFieldType::UInt32 },
    { "Rare4",       DbcFieldType::UInt32 },
    { "Rare5",       DbcFieldType::UInt32 },
    { "Uncommon1",   DbcFieldType::UInt32 },
    { "Uncommon2",   DbcFieldType::UInt32 },
    { "Uncommon3",   DbcFieldType::UInt32 },
    { "Uncommon4",   DbcFieldType::UInt32 },
    { "Uncommon5",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_rand_prop_points = {
    "RandPropPoints",
    rand_prop_points_fields,
    sizeof(rand_prop_points_fields) / sizeof(rand_prop_points_fields[0])
};

#endif
