#ifndef SCHEMA_MOVIE_H
#define SCHEMA_MOVIE_H

#include "dbc_schema.h"

static const DbcFieldDef movie_fields[] = {
    { "Id",        DbcFieldType::UInt32 },
    { "Filename",  DbcFieldType::String },
    { "Volume",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_movie = {
    "Movie",
    movie_fields,
    sizeof(movie_fields) / sizeof(movie_fields[0])
};

#endif
