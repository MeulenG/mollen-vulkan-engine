#ifndef SCHEMA_PAGE_TEXT_MATERIAL_H
#define SCHEMA_PAGE_TEXT_MATERIAL_H

#include "dbc_schema.h"

static const DbcFieldDef page_text_material_fields[] = {
    { "Id",    DbcFieldType::UInt32 },
    { "Name",  DbcFieldType::String },
};

static const DbcSchema schema_page_text_material = {
    "PageTextMaterial",
    page_text_material_fields,
    sizeof(page_text_material_fields) / sizeof(page_text_material_fields[0])
};

#endif
