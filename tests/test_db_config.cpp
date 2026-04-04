#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include "db_config.h"

static std::string WriteTempConfig(const std::string& content) {
    std::string path = std::tmpnam(nullptr);
    path += ".toml";
    FILE* f = fopen(path.c_str(), "w");
    REQUIRE(f != nullptr);
    fwrite(content.data(), 1, content.size(), f);
    fclose(f);
    return path;
}

TEST_CASE("LoadDbConfig parses valid config", "[config]") {
    std::string path = WriteTempConfig(R"(
[database]
dbname = "testdb"
user = "testuser"
password = "secret"
host = "127.0.0.1"
port = 5433
)");

    DbConfig cfg;
    REQUIRE(LoadDbConfig(path, cfg));
    CHECK(cfg.dbname == "testdb");
    CHECK(cfg.user == "testuser");
    CHECK(cfg.password == "secret");
    CHECK(cfg.host == "127.0.0.1");
    CHECK(cfg.port == 5433);

    std::remove(path.c_str());
}

TEST_CASE("LoadDbConfig uses defaults for optional fields", "[config]") {
    std::string path = WriteTempConfig(R"(
[database]
dbname = "mydb"
user = "admin"
)");

    DbConfig cfg;
    REQUIRE(LoadDbConfig(path, cfg));
    CHECK(cfg.dbname == "mydb");
    CHECK(cfg.user == "admin");
    CHECK(cfg.password == "");
    CHECK(cfg.host == "localhost");
    CHECK(cfg.port == 5432);

    std::remove(path.c_str());
}

TEST_CASE("LoadDbConfig fails on missing [database] section", "[config]") {
    std::string path = WriteTempConfig(R"(
[other]
key = "value"
)");

    DbConfig cfg;
    REQUIRE_FALSE(LoadDbConfig(path, cfg));

    std::remove(path.c_str());
}

TEST_CASE("LoadDbConfig fails on missing required fields", "[config]") {
    std::string path = WriteTempConfig(R"(
[database]
host = "localhost"
)");

    DbConfig cfg;
    REQUIRE_FALSE(LoadDbConfig(path, cfg));

    std::remove(path.c_str());
}

TEST_CASE("LoadDbConfig fails on nonexistent file", "[config]") {
    DbConfig cfg;
    REQUIRE_FALSE(LoadDbConfig("/nonexistent/path.toml", cfg));
}

TEST_CASE("LoadDbConfig fails on malformed TOML", "[config]") {
    std::string path = WriteTempConfig("this is not valid toml [[[");

    DbConfig cfg;
    REQUIRE_FALSE(LoadDbConfig(path, cfg));

    std::remove(path.c_str());
}
