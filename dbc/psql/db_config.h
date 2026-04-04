#ifndef DB_CONFIG_H
#define DB_CONFIG_H

#include <string>

struct DbConfig {
    std::string dbname;
    std::string user;
    std::string password;
    std::string host = "localhost";
    int port = 5432;
};

bool LoadDbConfig(const std::string& path, DbConfig& out);

#endif // DB_CONFIG_H
