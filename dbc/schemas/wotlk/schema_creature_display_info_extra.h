#ifndef SCHEMA_CREATURE_DISPLAY_INFO_EXTRA_H
#define SCHEMA_CREATURE_DISPLAY_INFO_EXTRA_H

#include "dbc_schema.h"

static const DbcFieldDef creature_display_info_extra_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "DisplayRaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "displayrace", "Visuals" },
    { "DisplaySexID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "displaysex", "Visuals" },
    { "SkinID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "skin" },
    { "FaceID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "face" },
    { "HairStyleID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "hairstyle" },
    { "HairColorID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "haircolor", "Visuals" },
    { "FacialHairID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "facialhair" },
    { "NPCItemDisplay1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay5", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay6", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay7", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay8", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay9", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay10", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "NPCItemDisplay11", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Flags", DbcFieldType::UInt32, DbcSemantic::Bitmask, nullptr, "Classification" },
    { "BakeName", DbcFieldType::String },
};

static const DbcSchema schema_creature_display_info_extra = {
    "CreatureDisplayInfoExtra",
    creature_display_info_extra_fields,
    sizeof(creature_display_info_extra_fields) / sizeof(creature_display_info_extra_fields[0])
};

#endif
