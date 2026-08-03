add_executable(mollen-wow-tools main.cpp)

# glfw is a shared DLL (see root CMakeLists - hot-reload requirement);
# deploy it next to the host exe.
add_custom_command(TARGET mollen-wow-tools POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:mollen-wow-tools>
)

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
# ----------------------------------------------------------------------------
# Runtime DLL copy (Windows + libpqxx)
# ----------------------------------------------------------------------------
# vcpkg's MSBuild integration adds installed/x64-windows/{lib,debug/lib} to the
# linker search path so pqxx.lib resolves at link time, but it does NOT copy
# the matching .dll files next to the .exe. Without that, launching produces
# "pqxx.dll not found".
#
# We discover the vcpkg installed-dir using common hints (VCPKG_ROOT env var,
# CMAKE_TOOLCHAIN_FILE pointing at vcpkg.cmake, sibling-of-repo layout) and
# add a post-build command to copy the runtime deps into $<TARGET_FILE_DIR>.
if(MVE_ENABLE_PSQL AND WIN32)
    set(_vcpkg_search_hints "")

    # 1. Explicit env var (preferred for portability across machines)
    if(DEFINED ENV{VCPKG_ROOT})
        list(APPEND _vcpkg_search_hints "$ENV{VCPKG_ROOT}/installed/x64-windows")
    endif()

    # 2. CMake toolchain file pointing at vcpkg.cmake
    if(CMAKE_TOOLCHAIN_FILE)
        get_filename_component(_tc_dir "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        list(APPEND _vcpkg_search_hints "${_tc_dir}/../../installed/x64-windows")
    endif()

    # 3. Common sibling-of-repo and per-user layouts
    list(APPEND _vcpkg_search_hints
        "${CMAKE_SOURCE_DIR}/../vcpkg/installed/x64-windows"
        "$ENV{USERPROFILE}/Desktop/vcpkg/installed/x64-windows"
        "$ENV{USERPROFILE}/source/repos/vcpkg/installed/x64-windows"
        "$ENV{USERPROFILE}/vcpkg/installed/x64-windows"
        "C:/vcpkg/installed/x64-windows"
        "C:/dev/vcpkg/installed/x64-windows")

    find_file(PQXX_RUNTIME_DLL
        NAMES pqxx.dll
        HINTS ${_vcpkg_search_hints}
        PATH_SUFFIXES bin
        NO_DEFAULT_PATH)

    if(PQXX_RUNTIME_DLL)
        get_filename_component(_dll_release_dir "${PQXX_RUNTIME_DLL}" DIRECTORY)
        get_filename_component(_vcpkg_installed "${_dll_release_dir}" DIRECTORY)
        set(_dll_debug_dir "${_vcpkg_installed}/debug/bin")

        message(STATUS "PSQL runtime DLLs: ${_vcpkg_installed}")

        # We restrict to DLLs whose names are identical in vcpkg's debug and
        # release bin dirs. zlib is intentionally excluded: vcpkg ships it as
        # zlib1.dll (release) and zlibd1.dll (debug), and libpq doesn't
        # require it for normal connections.
        set(_candidate_dlls
            pqxx.dll libpq.dll
            libssl-3-x64.dll libcrypto-3-x64.dll)

        # Pick out the ones that exist in BOTH dirs - anything missing from
        # either side is silently skipped to keep the build green if vcpkg
        # ships a slimmer config than expected.
        set(_copy_args "")
        foreach(_dll ${_candidate_dlls})
            if(EXISTS "${_dll_release_dir}/${_dll}" AND
               EXISTS "${_dll_debug_dir}/${_dll}")
                list(APPEND _copy_args
                    "$<IF:$<CONFIG:Debug>,${_dll_debug_dir},${_dll_release_dir}>/${_dll}")
            endif()
        endforeach()

        # We use a custom target with ALL (rather than add_custom_command's
        # POST_BUILD form) so the copy is checked on every build, even when
        # the .exe didn't relink. POST_BUILD only fires when MSBuild re-runs
        # the link step - meaning if the DLLs are deleted externally (IDE
        # clean, git clean -fdx, antivirus, etc.) and the .exe is still
        # up-to-date, the next incremental build won't restore them and
        # launching the .exe fails with "pqxx.dll not found".
        #
        # copy_if_different is idempotent and stat-only fast, so running it
        # every build is cheap.
        if(_copy_args)
            add_custom_target(mollen-wow-tools-runtime-dlls ALL
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${_copy_args}
                    "$<TARGET_FILE_DIR:mollen-wow-tools>"
                COMMENT "Staging libpqxx runtime DLLs next to mollen-wow-tools.exe")
            add_dependencies(mollen-wow-tools-runtime-dlls mollen-wow-tools)
        endif()
    else()
        message(WARNING
            "MVE_ENABLE_PSQL is ON but pqxx.dll could not be located.\n"
            "  Set VCPKG_ROOT (or CMAKE_TOOLCHAIN_FILE to vcpkg.cmake), then\n"
            "  reconfigure. Without this the .exe will fail to launch with\n"
            "  'pqxx.dll not found'.")
    endif()
endif()
