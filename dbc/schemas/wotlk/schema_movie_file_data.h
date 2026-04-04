#ifndef SCHEMA_MOVIE_FILE_DATA_H
#define SCHEMA_MOVIE_FILE_DATA_H

#include "dbc_schema.h"

static const DbcFieldDef movie_file_data_fields[] = {
    { "Id",         DbcFieldType::UInt32 },
    { "Resolution", DbcFieldType::UInt32 },
};

static const DbcSchema schema_movie_file_data = {
    "MovieFileData",
    movie_file_data_fields,
    sizeof(movie_file_data_fields) / sizeof(movie_file_data_fields[0])
};

#endif
