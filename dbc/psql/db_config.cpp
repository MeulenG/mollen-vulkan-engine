#include "db_config.h"

#include <cstdio>
#include <toml++/toml.hpp>

bool LoadDbConfig(const std::string& path, DbConfig& out)
{
    try {
        toml::table tbl = toml::parse_file(path);

        auto db = tbl["database"];
        if (!db) {
            fprintf(stderr, "Config missing [database] section\n");
            return false;
        }

        auto dbname = db["dbname"].value<std::string>();
        auto user   = db["user"].value<std::string>();
        if (!dbname || !user) {
            fprintf(stderr, "Config missing required fields: dbname, user\n");
            return false;
        }

        out.dbname   = *dbname;
        out.user     = *user;
        out.password = db["password"].value_or(std::string(""));
        out.host     = db["host"].value_or(std::string("localhost"));
        out.port     = db["port"].value_or(5432);

        return true;
    } catch (const toml::parse_error& err) {
        fprintf(stderr, "Config parse error: %s\n", err.what());
        return false;
    }
}
