add_library(mollen-engine SHARED
    core/window.cpp
    core/device.cpp
    core/swapchain.cpp
    core/renderer.cpp
    core/engine_state.cpp
    core/engine_api.cpp
    resources/pipeline.cpp
    resources/buffer.cpp
    resources/descriptor.cpp
    resources/image.cpp
    resources/texture_array.cpp
    resources/asset_manager.cpp
    systems/render_system.cpp
    systems/animation_system.cpp
    systems/editor_ui_system.cpp
    scene/mesh.cpp
    scene/camera.cpp
    scene/entity.cpp
    scene/scene.cpp
    animation/skeleton.cpp
    animation/animation_clip.cpp
    animation/animator.cpp
    core/offscreen_pass.cpp
    core/imgui_context.cpp
    formats/blp_loader.cpp
    formats/m2_loader.cpp
    formats/wdt_loader.cpp
    formats/adt_loader.cpp
    formats/wmo_loader.cpp
    scene/terrain_mesh.cpp
    scene/terrain_streamer.cpp
    scene/water_mesh.cpp
    scene/wmo_mesh.cpp
    scene/grass_scatter.cpp
    scene/light_cycle.cpp
    scene/player_controller.cpp
)

target_include_directories(mollen-engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Exported as a module DLL. Only the engine_* C entry points (marked with
# MVE_API / dllexport via MVE_ENGINE_BUILD) cross the boundary; everything
# below is baked into the DLL and kept PRIVATE so it does not propagate into
# the host exe (which would duplicate ImGui/GLFW global state).
target_compile_definitions(mollen-engine PRIVATE MVE_ENGINE_BUILD)

target_link_libraries(mollen-engine PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    imgui_lib
    mollen-dbc
)

# Deploy the engine DLL next to the host exe on every engine build. The host
# watches this file and hot-reloads when it changes, so deploying on an
# engine-only rebuild is what makes the reload loop work. (Generator
# expression for the app dir is resolved after all targets are defined, even
# though the engine subdirectory is processed before the app's.)
add_custom_command(TARGET mollen-engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:mollen-engine> $<TARGET_FILE_DIR:mollen-wow-tools>
)

# Compile GLSL shaders to SPIR-V
find_program(GLSLC glslc HINTS "$ENV{VULKAN_SDK}/Bin" "${Vulkan_GLSLC_EXECUTABLE}")

if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found. Ensure the Vulkan SDK is installed and VULKAN_SDK is set.")
endif()

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

file(GLOB SHADER_SOURCES
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.comp"
)

file(MAKE_DIRECTORY "${SHADER_OUTPUT_DIR}")

set(SHADER_OUTPUTS "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER_NAME}"
    )

    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

add_custom_target(compile-shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(mollen-engine compile-shaders)

# Make shader output directory available to consumers
target_compile_definitions(mollen-engine PUBLIC
    MVE_SHADER_DIR="${SHADER_OUTPUT_DIR}"
)

# Absolute path to the assets/ directory at the repo root. Consumers
# concatenate WoW-style relative paths (e.g. "World/Maps/Azeroth/...")
# onto this so the executable can find its assets regardless of CWD.
target_compile_definitions(mollen-engine PUBLIC
    MVE_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)
