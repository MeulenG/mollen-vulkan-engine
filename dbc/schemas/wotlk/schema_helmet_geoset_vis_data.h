#ifndef SCHEMA_HELMET_GEOSET_VIS_DATA_H
#define SCHEMA_HELMET_GEOSET_VIS_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef helmet_geoset_vis_data_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "HairGeoset",      DbcFieldType::UInt32 },
    { "Accessory1Geoset", DbcFieldType::UInt32 },
    { "Accessory2Geoset", DbcFieldType::UInt32 },
    { "Accessory3Geoset", DbcFieldType::UInt32 },
    { "Accessory4Geoset", DbcFieldType::UInt32 },
    { "Accessory5Geoset", DbcFieldType::UInt32 },
    { "Accessory6Geoset", DbcFieldType::UInt32 },
};

static const DbcSchema schema_helmet_geoset_vis_data = {
    "HelmetGeosetVisData",
    helmet_geoset_vis_data_fields,
    sizeof(helmet_geoset_vis_data_fields) / sizeof(helmet_geoset_vis_data_fields[0])
};

#endif
