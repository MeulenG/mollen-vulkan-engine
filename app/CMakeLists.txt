add_executable(mollen-wow-tools main.cpp)

# Hot-reload host: it loads the engine DLL at RUNTIME via LoadLibrary, so it
# deliberately does NOT link the engine import lib - implicit linking would
# make the OS lock the DLL at startup and prevent reloads. It only needs the
# engine DLL to have been built (and deployed next to the exe by the engine
# target's post-build copy).
add_dependencies(mollen-wow-tools mollen-engine)

# Launch with the repo root as the working directory under the VS debugger so
# the engine's relative asset paths (assets/World/...) resolve.
set_target_properties(mollen-wow-tools PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
