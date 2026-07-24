#include <catch2/catch_test_macros.hpp>
#include "psql_connector.h"
#include "db_config.h"

TEST_CASE("psql_connector starts disconnected", "[psql]") {
    psql_connector db;
    CHECK_FALSE(db.IsConnected());
}

TEST_CASE("psql_connector Disconnect is safe when not connected", "[psql]") {
    psql_connector db;
    REQUIRE_NOTHROW(db.Disconnect());
    CHECK_FALSE(db.IsConnected());
}

// Integration test - only runs if a real database is available.
// Set environment variable DBC_TEST_DB_CONFIG to a valid .toml config path to enable.
TEST_CASE("psql_connector integration: connect, query, disconnect", "[psql][integration]") {
    const char* config_path = std::getenv("DBC_TEST_DB_CONFIG");
    if (!config_path) {
        SKIP("Set DBC_TEST_DB_CONFIG=/path/to/config.toml to run integration tests");
    }

    DbConfig cfg;
    REQUIRE(LoadDbConfig(config_path, cfg));

    psql_connector db;
    REQUIRE(db.Connect(cfg));
    CHECK(db.IsConnected());

    // Simple query to verify the connection works
    pqxx::result res = db.Exec("SELECT 1 AS check_val");
    REQUIRE(res.size() == 1);
    CHECK(res[0][0].as<int>() == 1);

    db.Disconnect();
    CHECK_FALSE(db.IsConnected());
}
