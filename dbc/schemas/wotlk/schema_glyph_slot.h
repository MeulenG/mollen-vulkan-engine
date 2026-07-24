#ifndef SCHEMA_GLYPH_SLOT_H
#define SCHEMA_GLYPH_SLOT_H

#include "dbc_schema.h"

static const DbcFieldDef glyph_slot_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Type", DbcFieldType::UInt32 },
    { "Order", DbcFieldType::UInt32 },
};

static const DbcSchema schema_glyph_slot = {
    "GlyphSlot",
    glyph_slot_fields,
    sizeof(glyph_slot_fields) / sizeof(glyph_slot_fields[0])
};

#endif
