#ifndef SCHEMA_CREATURE_DISPLAY_INFO_EXTRA_H
#define SCHEMA_CREATURE_DISPLAY_INFO_EXTRA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_display_info_extra_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "DisplayRaceID",   DbcFieldType::UInt32 },
    { "DisplaySexID",    DbcFieldType::UInt32 },
    { "SkinID",          DbcFieldType::UInt32 },
    { "FaceID",          DbcFieldType::UInt32 },
    { "HairStyleID",     DbcFieldType::UInt32 },
    { "HairColorID",     DbcFieldType::UInt32 },
    { "FacialHairID",    DbcFieldType::UInt32 },
    { "NPCItemDisplay1", DbcFieldType::UInt32 },
    { "NPCItemDisplay2", DbcFieldType::UInt32 },
    { "NPCItemDisplay3", DbcFieldType::UInt32 },
    { "NPCItemDisplay4", DbcFieldType::UInt32 },
    { "NPCItemDisplay5", DbcFieldType::UInt32 },
    { "NPCItemDisplay6", DbcFieldType::UInt32 },
    { "NPCItemDisplay7", DbcFieldType::UInt32 },
    { "NPCItemDisplay8", DbcFieldType::UInt32 },
    { "NPCItemDisplay9", DbcFieldType::UInt32 },
    { "NPCItemDisplay10", DbcFieldType::UInt32 },
    { "NPCItemDisplay11", DbcFieldType::UInt32 },
    { "Flags",           DbcFieldType::UInt32 },
    { "BakeName",        DbcFieldType::String },
};

static const DbcSchema schema_creature_display_info_extra = {
    "CreatureDisplayInfoExtra",
    creature_display_info_extra_fields,
    sizeof(creature_display_info_extra_fields) / sizeof(creature_display_info_extra_fields[0])
};

#endif
