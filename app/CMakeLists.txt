add_executable(mollen-wow-tools main.cpp)

target_link_libraries(mollen-wow-tools PRIVATE
    mollen-engine
    mollen-dbc
    imgui_lib
)
