cmake_minimum_required(VERSION 3.20)
project(mollen-wow-tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(MVE_BUILD_DBC_TOOLS "Build dbc-extract CLI tool (requires mollen-storm + pqxx)" OFF)
option(MVE_BUILD_TESTS "Build unit tests" ON)
option(MVE_ENABLE_PSQL "Enable PostgreSQL support (requires libpq + libpqxx)" ON)

# =============================================================================
# Dependencies
# =============================================================================
include(FetchContent)

# Vulkan (must be installed via Vulkan SDK)
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

# mollen-storm (MPQ archive reading for dbc-extract)
if(MVE_BUILD_DBC_TOOLS)
    FetchContent_Declare(
        mollen_storm
        GIT_REPOSITORY https://github.com/MeulenG/mollen-storm.git
        GIT_TAG        master
    )
endif()

# Catch2 (testing)
if(MVE_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
endif()

# Make all declared dependencies available
set(FETCH_TARGETS glfw glm imgui tomlplusplus)
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

# PostgreSQL (optional)
if(MVE_ENABLE_PSQL)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(PQXX REQUIRED libpqxx)
    endif()
endif()

# =============================================================================
# Subdirectories
# =============================================================================
add_subdirectory(dbc)
add_subdirectory(engine)
add_subdirectory(app)

if(MVE_BUILD_TESTS AND MVE_ENABLE_PSQL)
    add_subdirectory(tests)
endif()
