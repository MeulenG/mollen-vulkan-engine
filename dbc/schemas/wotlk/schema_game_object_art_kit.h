#ifndef SCHEMA_GAME_OBJECT_ART_KIT_H
#define SCHEMA_GAME_OBJECT_ART_KIT_H

#include "dbc_schema.h"

static const DbcFieldDef game_object_art_kit_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "TextureVariation1", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "TextureVariation2", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "TextureVariation3", DbcFieldType::String, DbcSemantic::Default, nullptr, "Visuals" },
    { "AttachModel1", DbcFieldType::String },
    { "AttachModel2", DbcFieldType::String },
    { "AttachModel3", DbcFieldType::String },
    { "AttachModel4", DbcFieldType::String },
};

static const DbcSchema schema_game_object_art_kit = {
    "GameObjectArtKit",
    game_object_art_kit_fields,
    sizeof(game_object_art_kit_fields) / sizeof(game_object_art_kit_fields[0])
};

#endif
