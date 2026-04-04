#include "psql_connector.h"

bool psql_connector::Connect(const std::string& dbname, const std::string& user,
                             const std::string& password, const std::string& host, int port)
{
    try {
        std::string connection_str =
            "dbname=" + dbname +
            " user=" + user +
            " password=" + password +
            " host=" + host +
            " port=" + std::to_string(port);

        conn_ = std::make_unique<pqxx::connection>(connection_str);
        return conn_->is_open();
    } catch (const pqxx::sql_error& e) {
        fprintf(stderr, "SQL error: %s (SQLSTATE %s)\n", e.what(), e.sqlstate().c_str());
        conn_.reset();
        return false;
    } catch (const std::exception& e) {
        fprintf(stderr, "Connection error: %s\n", e.what());
        conn_.reset();
        return false;
    }
}

bool psql_connector::Connect(const DbConfig& config)
{
    return Connect(config.dbname, config.user, config.password, config.host, config.port);
}

void psql_connector::Disconnect()
{
    conn_.reset();
}

bool psql_connector::IsConnected() const
{
    return conn_ && conn_->is_open();
}

pqxx::result psql_connector::Exec(const std::string& sql)
{
    pqxx::work txn(*conn_);
    pqxx::result res = txn.exec(sql);
    txn.commit();
    return res;
}

pqxx::connection& psql_connector::GetConnection()
{
    return *conn_;
}
