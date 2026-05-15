#ifndef SCHEMA_WORLD_MAP_TRANSFORMS_H
#define SCHEMA_WORLD_MAP_TRANSFORMS_H

#include "dbc_schema.h"

static const DbcFieldDef world_map_transforms_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "MapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "map" },
    { "RegionMinX",      DbcFieldType::Float  },
    { "RegionMinY",      DbcFieldType::Float  },
    { "RegionMaxX",      DbcFieldType::Float  },
    { "RegionMaxY",      DbcFieldType::Float  },
    { "NewMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "newmap" },
    { "RegionOffsetX",   DbcFieldType::Float  },
    { "RegionOffsetY",   DbcFieldType::Float  },
    { "NewDungeonMapID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "newdungeonmap" },
};

static const DbcSchema schema_world_map_transforms = {
    "WorldMapTransforms",
    world_map_transforms_fields,
    sizeof(world_map_transforms_fields) / sizeof(world_map_transforms_fields[0])
};

#endif
