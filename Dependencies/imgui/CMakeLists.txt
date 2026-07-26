cmake_minimum_required(VERSION 3.20)

project(imgui LANGUAGES CXX)

add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
    misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
)

target_link_libraries(imgui
    PUBLIC
        glfw
        glad
)

target_compile_features(imgui PUBLIC cxx_std_20)
