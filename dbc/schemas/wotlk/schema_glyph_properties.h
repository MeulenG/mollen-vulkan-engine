#ifndef SCHEMA_GLYPH_PROPERTIES_H
#define SCHEMA_GLYPH_PROPERTIES_H

#include "dbc_schema.h"

static const DbcFieldDef glyph_properties_fields[] = {
    { "Id",           DbcFieldType::UInt32 },
    { "SpellID",      DbcFieldType::UInt32 },
    { "TypeFlags",    DbcFieldType::UInt32 },
    { "SpellIconID",  DbcFieldType::UInt32 },
};

static const DbcSchema schema_glyph_properties = {
    "GlyphProperties",
    glyph_properties_fields,
    sizeof(glyph_properties_fields) / sizeof(glyph_properties_fields[0])
};

#endif
