#ifndef SCHEMA_WORLD_SAFE_LOCS_H
#define SCHEMA_WORLD_SAFE_LOCS_H

#include "dbc_schema.h"

static const DbcFieldDef world_safe_locs_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "MapID",     DbcFieldType::UInt32 },
    { "X",         DbcFieldType::Float  },
    { "Y",         DbcFieldType::Float  },
    { "Z",         DbcFieldType::Float  },
    { "Name_enUS", DbcFieldType::String },
    { "Name_koKR", DbcFieldType::String },
    { "Name_frFR", DbcFieldType::String },
    { "Name_deDE", DbcFieldType::String },
    { "Name_enCN", DbcFieldType::String },
    { "Name_zhTW", DbcFieldType::String },
    { "Name_esES", DbcFieldType::String },
    { "Name_esMX", DbcFieldType::String },
    { "Name_ruRU", DbcFieldType::String },
    { "Name_jaJP", DbcFieldType::String },
    { "Name_ptPT", DbcFieldType::String },
    { "Name_itIT", DbcFieldType::String },
    { "Name_Unk1", DbcFieldType::String },
    { "Name_Unk2", DbcFieldType::String },
    { "Name_Unk3", DbcFieldType::String },
    { "Name_Unk4", DbcFieldType::String },
    { "NameFlags", DbcFieldType::UInt32 },
};

static const DbcSchema schema_world_safe_locs = {
    "WorldSafeLocs",
    world_safe_locs_fields,
    sizeof(world_safe_locs_fields) / sizeof(world_safe_locs_fields[0])
};

#endif
