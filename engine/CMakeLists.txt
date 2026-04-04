add_library(mollen-engine STATIC
    core/window.cpp
)

target_include_directories(mollen-engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mollen-engine PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
)
