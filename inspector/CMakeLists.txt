add_executable(mollen-inspector
    main.cpp
)

target_link_libraries(mollen-inspector PRIVATE
    imgui_lib_opengl3
    glfw
)

set_target_properties(mollen-inspector PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
