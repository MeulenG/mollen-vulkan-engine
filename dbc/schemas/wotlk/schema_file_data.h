#ifndef SCHEMA_FILE_DATA_H
#define SCHEMA_FILE_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef file_data_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Filename",  DbcFieldType::String },
    { "FilePath",  DbcFieldType::String },
};

static const DbcSchema schema_file_data = {
    "FileData",
    file_data_fields,
    sizeof(file_data_fields) / sizeof(file_data_fields[0])
};

#endif
