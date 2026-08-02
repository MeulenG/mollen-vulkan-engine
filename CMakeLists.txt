cmake_minimum_required(VERSION 3.20)
project(mollen-wow-tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Hot-reload (HOTRELOAD_PLAN.md) requires the host exe and the future engine
# module DLL to share ONE C runtime, so heap allocations made by module code
# survive a FreeLibrary/LoadLibrary cycle. Lock every target to the dynamic
# (DLL) CRT explicitly rather than relying on the CMake default, so the host
# and module can never drift onto different runtimes. Set before any target
# (including FetchContent deps) is created so they all inherit it.
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Options
option(MVE_BUILD_DBC_TOOLS "Build dbc-extract CLI tool (requires mollen-storm + pqxx)" OFF)
option(MVE_BUILD_TESTS "Build unit tests" ON)
option(MVE_BUILD_INSPECTOR "Build mollen-inspector format viewer" ON)
option(MVE_ENABLE_PSQL "Enable PostgreSQL support (requires libpq)" OFF)

# Dependencies
include(FetchContent)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# Dear ImGui (docking branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)

# tomlplusplus (for DBC config)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)

# mollen-storm (MPQ archive reading for asset-extract)
if(MVE_BUILD_DBC_TOOLS)
    FetchContent_Declare(
        mollen_storm
        GIT_REPOSITORY https://github.com/MeulenG/mollen-storm.git
        GIT_TAG        master
    )
endif()

# mollen-wow-dbc (DBC parsing, schemas, writer, psql import). The engine
# consumes the library only - the repo's own CLI and tests stay off in
# embedded builds. Its psql support tracks MVE_ENABLE_PSQL.
set(MOLLEN_DBC_ENABLE_PSQL ${MVE_ENABLE_PSQL} CACHE BOOL "" FORCE)
FetchContent_Declare(
    mollen_wow_dbc
    GIT_REPOSITORY https://github.com/MeulenG/mollen-wow-dbc.git
    GIT_TAG        master
)

# Catch2 (testing)
if(MVE_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
endif()

# Make all declared dependencies available
set(FETCH_TARGETS glfw glm imgui tomlplusplus mollen_wow_dbc)
if(MVE_BUILD_DBC_TOOLS)
    list(APPEND FETCH_TARGETS mollen_storm)
endif()
if(MVE_BUILD_TESTS)
    list(APPEND FETCH_TARGETS Catch2)
endif()
FetchContent_MakeAvailable(${FETCH_TARGETS})

# Dear ImGui doesn't have a CMakeLists.txt, so we build it manually
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC Vulkan::Vulkan glfw)

# Separate ImGui target for the inspector (OpenGL3 backend, no Vulkan needed)
if(MVE_BUILD_INSPECTOR)
    find_package(OpenGL REQUIRED)
    add_library(imgui_lib_opengl3 STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui_lib_opengl3 PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_lib_opengl3 PUBLIC glfw OpenGL::GL)
endif()

# PostgreSQL (optional)
if(MVE_ENABLE_PSQL)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(PQXX REQUIRED libpqxx)
    endif()
endif()

# Engine targets link the historical `mollen-dbc` name; the actual library
# now comes from the mollen-wow-dbc repo via FetchContent.
add_library(mollen-dbc ALIAS mollen-dbc-schemas)

if(MVE_BUILD_DBC_TOOLS)
    # Asset extraction tool (M2, skin, BLP files from MPQ). The dbc-extract
    # CLI lives in mollen-wow-dbc now - build that repo standalone for it.
    add_executable(asset-extract tools/asset_extract.cpp)
    target_link_libraries(asset-extract PRIVATE mollen-storm-mpq)
endif()

add_subdirectory(engine)
add_subdirectory(app)

if(MVE_BUILD_INSPECTOR)
    add_subdirectory(inspector)
endif()

if(MVE_BUILD_TESTS AND MVE_ENABLE_PSQL)
    add_subdirectory(tests)
endif()
