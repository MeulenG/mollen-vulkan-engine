add_executable(mollen-wow-tools main.cpp)

# The host only calls the engine's C entry points. It links mollen-engine for
# the import lib (phase 4); it deliberately does NOT link imgui_lib/mollen-dbc
# - those live inside the engine DLL. Including them here would duplicate their
# global state into the host.
target_link_libraries(mollen-wow-tools PRIVATE
    mollen-engine
)

# The engine is a DLL now; Windows resolves it from the exe's own directory at
# runtime, so drop a fresh copy next to the host exe after each build.
add_custom_command(TARGET mollen-wow-tools POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:mollen-engine> $<TARGET_FILE_DIR:mollen-wow-tools>
)

# Launch the executable with the repo root as the working directory when
# started through Visual Studio's debugger. Asset paths are relative to the
# repo root (e.g. "assets/World/Maps/Azeroth/..."), so without this the
# debugger leaves CWD at $(OutDir) and every asset load silently fails.
set_target_properties(mollen-wow-tools PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
