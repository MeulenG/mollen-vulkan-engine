#ifndef SCHEMA_WORLD_MAP_CONTINENT_H
#define SCHEMA_WORLD_MAP_CONTINENT_H

#include "dbc_schema.h"

static const DbcFieldDef world_map_continent_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "LeftBoundary", DbcFieldType::UInt32 },
    { "RightBoundary", DbcFieldType::UInt32 },
    { "TopBoundary", DbcFieldType::UInt32 },
    { "BottomBoundary", DbcFieldType::UInt32 },
    { "ContinentOffsetX", DbcFieldType::Float },
    { "ContinentOffsetY", DbcFieldType::Float },
    { "Scale", DbcFieldType::Float },
    { "TaxiMinX", DbcFieldType::Float },
    { "TaxiMinY", DbcFieldType::Float },
    { "TaxiMaxX", DbcFieldType::Float },
    { "TaxiMaxY", DbcFieldType::Float },
    { "WorldMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldmap" },
};

static const DbcSchema schema_world_map_continent = {
    "WorldMapContinent",
    world_map_continent_fields,
    sizeof(world_map_continent_fields) / sizeof(world_map_continent_fields[0])
};

#endif
