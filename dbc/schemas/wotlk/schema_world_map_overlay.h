#ifndef SCHEMA_WORLD_MAP_OVERLAY_H
#define SCHEMA_WORLD_MAP_OVERLAY_H

#include "dbc_schema.h"

static const DbcFieldDef world_map_overlay_fields[] = {
    { "Id",             DbcFieldType::UInt32 },
    { "WorldMapAreaID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "worldmaparea" },
    { "AreaTableID1",   DbcFieldType::UInt32 },
    { "AreaTableID2",   DbcFieldType::UInt32 },
    { "AreaTableID3",   DbcFieldType::UInt32 },
    { "AreaTableID4",   DbcFieldType::UInt32 },
    { "MapPointX",      DbcFieldType::UInt32 },
    { "MapPointY",      DbcFieldType::UInt32 },
    { "TextureName",    DbcFieldType::String },
    { "TextureWidth",   DbcFieldType::UInt32 },
    { "TextureHeight",  DbcFieldType::UInt32 },
    { "OffsetX",        DbcFieldType::UInt32 },
    { "OffsetY",        DbcFieldType::UInt32 },
    { "HitRectTop",     DbcFieldType::UInt32 },
    { "HitRectLeft",    DbcFieldType::UInt32 },
    { "HitRectBottom",  DbcFieldType::UInt32 },
    { "HitRectRight",   DbcFieldType::UInt32 },
};

static const DbcSchema schema_world_map_overlay = {
    "WorldMapOverlay",
    world_map_overlay_fields,
    sizeof(world_map_overlay_fields) / sizeof(world_map_overlay_fields[0])
};

#endif
