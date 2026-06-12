add_library(mollen-engine STATIC
    core/window.cpp
    core/device.cpp
    core/swapchain.cpp
    core/renderer.cpp
    resources/pipeline.cpp
    resources/buffer.cpp
    resources/descriptor.cpp
    resources/image.cpp
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
    formats/chunk_handler.cpp
    formats/wdt_loader.cpp
    formats/adt_loader.cpp
    scene/terrain_mesh.cpp
)

target_include_directories(mollen-engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mollen-engine PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    imgui_lib
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
