#ifndef DBC_GENERATOR_H
#define DBC_GENERATOR_H

#include <string>
#include <filesystem>

#include "dbc_file.h"
#include "dbc_schema.h"

bool GenerateHeader(const DbcFile& dbc, const DbcSchema* schema,
                    const std::filesystem::path& output_dir,
                    bool quiet = false);

#endif // DBC_GENERATOR_H
