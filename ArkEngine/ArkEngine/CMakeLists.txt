cmake_minimum_required(VERSION 3.20)
project(ArkEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/Vendor/GLAD/include
    ${CMAKE_SOURCE_DIR}/Vendor/GLFW/include
    ${CMAKE_SOURCE_DIR}/Vendor/glm
    ${CMAKE_SOURCE_DIR}/Vendor/assimp/include
    ${CMAKE_SOURCE_DIR}/Vendor/stb_image
    ${CMAKE_SOURCE_DIR}/Vendor/lodepng/include
    ${CMAKE_SOURCE_DIR}/Vendor/tinyexr
    ${CMAKE_SOURCE_DIR}/Vendor/tinyobjloader
    ${CMAKE_SOURCE_DIR}/Vendor/nlohmann_json/include
    ${CMAKE_SOURCE_DIR}/Vendor/magic_enum/include
    ${CMAKE_SOURCE_DIR}/Vendor/Vulkan/Include
    ${CMAKE_SOURCE_DIR}/Vendor/VulkanMemoryAllocator/include
    ${CMAKE_SOURCE_DIR}/Vendor/VkBootstrap
    ${CMAKE_SOURCE_DIR}/Vendor/entt
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/include
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/include/imgui
    ${CMAKE_SOURCE_DIR}/Vendor/compressonator/include
)

# Create executable first
add_executable(${PROJECT_NAME})

# Source files
file(GLOB_RECURSE ENGINE_SOURCES
    "src/*.cpp"
    "src/*.c"
    "Vendor/GLAD/src/*.c"
    "Vendor/stb_image/*.cpp"
    "Vendor/tinyexr/*.cpp"
)

# lodepng: only the library sources (exclude benchmarks/tests requiring SDL2, etc.)
list(APPEND ENGINE_SOURCES
    "Vendor/lodepng/include/lodepng/lodepng.cpp"
    "Vendor/lodepng/include/lodepng/lodepng_util.cpp"
)

# ImGui core + only the backends we actually use (GLFW + OpenGL3).
set(IMGUI_SOURCES
    "Vendor/imgui/include/imgui/imgui.cpp"
    "Vendor/imgui/include/imgui/imgui_draw.cpp"
    "Vendor/imgui/include/imgui/imgui_tables.cpp"
    "Vendor/imgui/include/imgui/imgui_widgets.cpp"
    "Vendor/imgui/include/imgui/imgui_demo.cpp"
    "Vendor/imgui/include/imgui/backends/imgui_impl_glfw.cpp"
    "Vendor/imgui/include/imgui/backends/imgui_impl_opengl3.cpp"
)

set(SOURCES ${ENGINE_SOURCES} ${IMGUI_SOURCES})

# Add source files to target
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Link libraries
# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# Vulkan (optional - only if building with Vulkan support)
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VULKAN_ENABLED)
endif()

# Assimp
find_package(assimp QUIET)
if(assimp_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
else()
    message(WARNING "Assimp not found - model loading may not work")
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32)
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()

if(APPLE)
    # macOS-specific settings
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY})
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX- /std:c++17)
    # Link filesystem library on MSVC (C++17 filesystem support)
    target_link_libraries(${PROJECT_NAME} PRIVATE filesystem)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
    # Link filesystem library on GCC/Clang (if needed)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
