#ifndef SCHEMA_GAME_OBJECT_DISPLAY_INFO_H
#define SCHEMA_GAME_OBJECT_DISPLAY_INFO_H

#include "dbc_schema.h"

static const DbcFieldDef game_object_display_info_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "ModelName", DbcFieldType::String },
    { "Sound1", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound2", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound3", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound4", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound5", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound6", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound7", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound8", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound9", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "Sound10", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Visuals" },
    { "GeoBoxMinX", DbcFieldType::Float },
    { "GeoBoxMinY", DbcFieldType::Float },
    { "GeoBoxMinZ", DbcFieldType::Float },
    { "GeoBoxMaxX", DbcFieldType::Float },
    { "GeoBoxMaxY", DbcFieldType::Float },
    { "GeoBoxMaxZ", DbcFieldType::Float },
    { "ObjectEffectPackageID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "objecteffectpackage" },
};

static const DbcSchema schema_game_object_display_info = {
    "GameObjectDisplayInfo",
    game_object_display_info_fields,
    sizeof(game_object_display_info_fields) / sizeof(game_object_display_info_fields[0])
};

#endif
