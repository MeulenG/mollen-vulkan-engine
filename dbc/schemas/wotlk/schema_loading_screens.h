#ifndef SCHEMA_LOADING_SCREENS_H
#define SCHEMA_LOADING_SCREENS_H

#include "dbc_schema.h"

static const DbcFieldDef loading_screens_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "Name", DbcFieldType::String, DbcSemantic::Default, nullptr, "Identity" },
    { "FileName", DbcFieldType::String },
    { "HasWideScreen", DbcFieldType::UInt32 },
};

static const DbcSchema schema_loading_screens = {
    "LoadingScreens",
    loading_screens_fields,
    sizeof(loading_screens_fields) / sizeof(loading_screens_fields[0])
};

#endif
