add_executable(dbc-tests
    test_db_config.cpp
    test_psql_connector.cpp
    test_enum_registry.cpp
)

target_link_libraries(dbc-tests PRIVATE
    mollen-dbc
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(dbc-tests
    DL_PATHS $<TARGET_FILE_DIR:dbc-tests>
    DISCOVERY_MODE PRE_TEST
)
