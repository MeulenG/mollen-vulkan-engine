#ifndef SCHEMA_CREATURE_MOVEMENT_INFO_H
#define SCHEMA_CREATURE_MOVEMENT_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef creature_movement_info_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "SmoothFacingTimer", DbcFieldType::Float },
};

static const DbcSchema schema_creature_movement_info = {
    "CreatureMovementInfo",
    creature_movement_info_fields,
    sizeof(creature_movement_info_fields) / sizeof(creature_movement_info_fields[0])
};

#endif
