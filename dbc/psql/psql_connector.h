#ifndef PSQL_CONNECTOR_H
#define PSQL_CONNECTOR_H

#include <string>
#include <memory>
#include <pqxx/pqxx>
#include "db_config.h"

class psql_connector
{
private:
    std::unique_ptr<pqxx::connection> conn_;

public:
    bool Connect(const std::string& dbname, const std::string& user,
                 const std::string& password, const std::string& host, int port);
    bool Connect(const DbConfig& config);
    void Disconnect();
    bool IsConnected() const;

    pqxx::result Exec(const std::string& sql);

    pqxx::connection& GetConnection();
};

#endif // PSQL_CONNECTOR_H
