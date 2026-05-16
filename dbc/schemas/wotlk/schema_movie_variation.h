#ifndef SCHEMA_MOVIE_VARIATION_H
#define SCHEMA_MOVIE_VARIATION_H

#include "dbc_schema.h"

static const DbcFieldDef movie_variation_fields[] = {
    { "Id", DbcFieldType::UInt32, DbcSemantic::Default, nullptr, "Identity" },
    { "MovieID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "movie" },
    { "FileDataID", DbcFieldType::UInt32, DbcSemantic::ForeignKey, "filedata" },
};

static const DbcSchema schema_movie_variation = {
    "MovieVariation",
    movie_variation_fields,
    sizeof(movie_variation_fields) / sizeof(movie_variation_fields[0])
};

#endif
