set(DBC_SOURCES
    dbc_file.cpp
    dbc_writer.cpp
    schema_registry.cpp
    dbc_generator.cpp
)

if(MVE_ENABLE_PSQL)
    list(APPEND DBC_SOURCES
        psql/db_config.cpp
        psql/psql_connector.cpp
        psql/dbc_db_import.cpp
    )
endif()

add_library(mollen-dbc STATIC ${DBC_SOURCES})

target_include_directories(mollen-dbc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/psql
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/wotlk
)

if(MVE_ENABLE_PSQL)
    target_link_libraries(mollen-dbc PUBLIC tomlplusplus::tomlplusplus pqxx)
    target_compile_definitions(mollen-dbc PUBLIC MVE_ENABLE_PSQL)
else()
    target_link_libraries(mollen-dbc PUBLIC tomlplusplus::tomlplusplus)
endif()

# dbc-extract CLI tool
if(MVE_BUILD_DBC_TOOLS)
    add_executable(dbc-extract tools/main.cpp)
    target_link_libraries(dbc-extract PRIVATE mollen-dbc mollen-storm-mpq)
    target_compile_definitions(dbc-extract PRIVATE MOLLEN_STORM_HAS_MPQ)

    if(MVE_ENABLE_PSQL)
        target_compile_definitions(dbc-extract PRIVATE MVE_ENABLE_PSQL)
    endif()

    # Asset extraction tool (M2, skin, BLP files from MPQ)
    add_executable(asset-extract tools/asset_extract.cpp)
    target_link_libraries(asset-extract PRIVATE mollen-storm-mpq)
endif()
