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

# glfw is a shared DLL (see root CMakeLists); deploy it next to the exe.
add_custom_command(TARGET mollen-inspector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:mollen-inspector>
)
