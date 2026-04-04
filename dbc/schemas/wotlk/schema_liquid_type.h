#ifndef SCHEMA_LIQUID_TYPE_H
#define SCHEMA_LIQUID_TYPE_H

#include "dbc_schema.h"

static const DbcFieldDef liquid_type_fields[] = {
    { "Id",              DbcFieldType::UInt32 },
    { "Name",            DbcFieldType::String },
    { "Flags",           DbcFieldType::UInt32 },
    { "Type",            DbcFieldType::UInt32 },
    { "SoundID",         DbcFieldType::UInt32 },
    { "SpellID",         DbcFieldType::UInt32 },
    { "MaxDarkenDepth",  DbcFieldType::Float  },
    { "FogDarkenIntensity", DbcFieldType::Float },
    { "AmbDarkenIntensity", DbcFieldType::Float },
    { "DirDarkenIntensity", DbcFieldType::Float },
    { "LightID",         DbcFieldType::UInt32 },
    { "ParticleScale",   DbcFieldType::Float  },
    { "ParticleMovement", DbcFieldType::UInt32 },
    { "ParticleTexSlots", DbcFieldType::UInt32 },
    { "MaterialID",      DbcFieldType::UInt32 },
    { "Texture1",        DbcFieldType::String },
    { "Texture2",        DbcFieldType::String },
    { "Texture3",        DbcFieldType::String },
    { "Texture4",        DbcFieldType::String },
    { "Texture5",        DbcFieldType::String },
    { "Texture6",        DbcFieldType::String },
    { "Color1",          DbcFieldType::UInt32 },
    { "Color2",          DbcFieldType::UInt32 },
    { "Float1",          DbcFieldType::Float  },
    { "Float2",          DbcFieldType::Float  },
    { "Float3",          DbcFieldType::Float  },
    { "Float4",          DbcFieldType::Float  },
    { "Float5",          DbcFieldType::Float  },
    { "Float6",          DbcFieldType::Float  },
    { "Float7",          DbcFieldType::Float  },
    { "Float8",          DbcFieldType::Float  },
    { "Float9",          DbcFieldType::Float  },
    { "Float10",         DbcFieldType::Float  },
    { "Float11",         DbcFieldType::Float  },
    { "Float12",         DbcFieldType::Float  },
    { "Float13",         DbcFieldType::Float  },
    { "Float14",         DbcFieldType::Float  },
    { "Float15",         DbcFieldType::Float  },
    { "Float16",         DbcFieldType::Float  },
    { "Float17",         DbcFieldType::Float  },
    { "Float18",         DbcFieldType::Float  },
    { "Int1",            DbcFieldType::UInt32 },
    { "Int2",            DbcFieldType::UInt32 },
    { "Int3",            DbcFieldType::UInt32 },
    { "Int4",            DbcFieldType::UInt32 },
};

static const DbcSchema schema_liquid_type = {
    "LiquidType",
    liquid_type_fields,
    sizeof(liquid_type_fields) / sizeof(liquid_type_fields[0])
};

#endif
