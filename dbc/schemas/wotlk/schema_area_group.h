#ifndef SCHEMA_AREA_GROUP_H
#define SCHEMA_AREA_GROUP_H

#include "dbc_schema.h"

static const DbcFieldDef area_group_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "AreaID1", DbcFieldType::UInt32 },
    { "AreaID2", DbcFieldType::UInt32 },
    { "AreaID3", DbcFieldType::UInt32 },
    { "AreaID4", DbcFieldType::UInt32 },
    { "AreaID5", DbcFieldType::UInt32 },
    { "AreaID6", DbcFieldType::UInt32 },
    { "NextGroup", DbcFieldType::UInt32 },
};

static const DbcSchema schema_area_group = {
    "AreaGroup",
    area_group_fields,
    sizeof(area_group_fields) / sizeof(area_group_fields[0])
};

#endif
