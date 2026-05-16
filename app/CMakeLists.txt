add_executable(mollen-wow-tools main.cpp)

target_link_libraries(mollen-wow-tools PRIVATE
    mollen-engine
    mollen-dbc
    imgui_lib
)

# Launch the executable with the repo root as the working directory when
# started through Visual Studio's debugger. Asset paths are relative to the
# repo root (e.g. "assets/World/Maps/Azeroth/..."), so without this the
# debugger leaves CWD at $(OutDir) and every asset load silently fails.
set_target_properties(mollen-wow-tools PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
