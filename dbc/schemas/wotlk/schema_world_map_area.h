#ifndef SCHEMA_WORLD_MAP_AREA_H
#define SCHEMA_WORLD_MAP_AREA_H

#include "dbc_schema.h"

static const DbcFieldDef world_map_area_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "AreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "areatable" },
    { "AreaName", DbcFieldType::String },
    { "LocLeft", DbcFieldType::Float },
    { "LocRight", DbcFieldType::Float },
    { "LocTop", DbcFieldType::Float },
    { "LocBottom", DbcFieldType::Float },
    { "DisplayMapID", DbcFieldType::Int32, DbcSemantic::ForeignKey, "displaymap", "Visuals" },
    { "DefaultDungeonFloor", DbcFieldType::Int32 },
    { "ParentWorldMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "parentworldmap" },
};

static const DbcSchema schema_world_map_area = {
    "WorldMapArea",
    world_map_area_fields,
    sizeof(world_map_area_fields) / sizeof(world_map_area_fields[0])
};

#endif
