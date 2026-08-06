cmake_minimum_required(VERSION 3.22)
project(Themepark VERSION 0.1.0 LANGUAGES C CXX)

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# Use bundled libraries
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "" FORCE)
set(ASSIMP_USE_SYSTEM_ZLIB OFF CACHE BOOL "" FORCE)

# OpenGL configuration
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

add_subdirectory(Dependencies/glad)
add_subdirectory(Dependencies/glfw)
add_subdirectory(Dependencies/glm)
add_subdirectory(Dependencies/imgui)
add_subdirectory(Dependencies/assimp)

if(WIN32)
    message(STATUS "Windows build")
elseif(APPLE)
    message(STATUS "macOS build")
elseif(UNIX)
    message(STATUS "Linux build")
endif()

set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glad")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

add_executable(${PROJECT_NAME})

if (MSVC)
    target_compile_options(${PROJECT_NAME}
      PRIVATE
        /W4
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Release>:/O2>

    )
else()
    target_compile_options(${PROJECT_NAME}
      PRIVATE
        -pedantic-errors
        -Wall
        # -Weffc++
        -Wextra
        -Wconversion
        # -Wsign-conversion
        # -Werror
        $<$<CONFIG:Debug>:-ggdb -O0>
        $<$<CONFIG:Release>:-O3>
    )
endif()

target_sources(${PROJECT_NAME}
  PRIVATE
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/collision.cpp
)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${CMAKE_SOURCE_DIR}/Dependencies/stb
)

add_library(projectHeaders INTERFACE)
target_include_directories(projectHeaders INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    include
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    OpenGL::GL
    glfw
    glad
    glm
    ${CMAKE_DL_LIBS}  # Required for GLAD on some systems
    projectHeaders
    stb
    imgui
    assimp
)

# Compiler and output settings
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
