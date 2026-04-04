#ifndef SCHEMA_MOVIE_VARIATION_H
#define SCHEMA_MOVIE_VARIATION_H

#include "dbc_schema.h"

static const DbcFieldDef movie_variation_fields[] = {
    { "Id",            DbcFieldType::UInt32 },
    { "MovieID",       DbcFieldType::UInt32 },
    { "FileDataID",    DbcFieldType::UInt32 },
};

static const DbcSchema schema_movie_variation = {
    "MovieVariation",
    movie_variation_fields,
    sizeof(movie_variation_fields) / sizeof(movie_variation_fields[0])
};

#endif
