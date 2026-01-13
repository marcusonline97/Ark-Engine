cmake_minimum_required(VERSION 3.20)
project(ArkEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional audio backends for the editor music player.
option(ARK_ENABLE_FMOD "Enable FMOD music backend (requires FMOD libs present)" OFF)
option(ARK_ENABLE_SOLOUD "Enable SoLoud music backend (requires SoLoud headers/libs present)" OFF)

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
    ${CMAKE_SOURCE_DIR}/Vendor/compressonator/include
)

# Create executable first
add_executable(${PROJECT_NAME})

# Backend availability (compile-time)
set(ARK_HAS_FMOD 0)
set(ARK_HAS_SOLOUD 0)

# Source files (add your source files here)
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
    "Vendor/glad/src/*.c"
    "Vendor/stb_image/*.cpp"
    "Vendor/lodepng/include/lodepng/*.cpp"
    "Vendor/tinyexr/*.cpp"
    "Vendor/imgui/include/imgui/*.cpp"
)

# Add source files to target
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# --- Optional audio backend wiring ---
if(ARK_ENABLE_FMOD)
    include_directories(${CMAKE_SOURCE_DIR}/Vendor/fmod/include)

    # Try common FMOD library names + typical install layouts.
    find_library(FMOD_LIBRARY
        NAMES fmod fmodL fmod64 fmodstudio fmodstudioL
        PATHS
            ${CMAKE_SOURCE_DIR}/Vendor/fmod/lib
            ${CMAKE_SOURCE_DIR}/Vendor/fmod/lib/x64
            $ENV{FMOD_DIR}
            $ENV{FMOD_DIR}/api/core/lib
            $ENV{FMOD_DIR}/api/core/lib/x86_64
            $ENV{FMOD_DIR}/api/core/lib/x64
    )

    if(FMOD_LIBRARY)
        message(STATUS "FMOD library: ${FMOD_LIBRARY}")
        set(ARK_HAS_FMOD 1)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${FMOD_LIBRARY})
    else()
        message(WARNING "ARK_ENABLE_FMOD is ON but FMOD library not found. Building without FMOD backend.")
    endif()
endif()

if(ARK_ENABLE_SOLOUD)
    find_path(SOLOUD_INCLUDE_DIR
        NAMES soloud.h
        PATHS
            ${CMAKE_SOURCE_DIR}/Vendor/soloud/include
            $ENV{SOLOUD_DIR}
            $ENV{SOLOUD_DIR}/include
    )

    find_library(SOLOUD_LIBRARY
        NAMES soloud
        PATHS
            ${CMAKE_SOURCE_DIR}/Vendor/soloud/lib
            ${CMAKE_SOURCE_DIR}/Vendor/soloud/lib/x64
            $ENV{SOLOUD_DIR}
            $ENV{SOLOUD_DIR}/lib
    )

    if(SOLOUD_INCLUDE_DIR AND SOLOUD_LIBRARY)
        message(STATUS "SoLoud include: ${SOLOUD_INCLUDE_DIR}")
        message(STATUS "SoLoud library: ${SOLOUD_LIBRARY}")
        include_directories(${SOLOUD_INCLUDE_DIR})
        set(ARK_HAS_SOLOUD 1)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SOLOUD_LIBRARY})
    else()
        message(WARNING "ARK_ENABLE_SOLOUD is ON but SoLoud was not found. Building without SoLoud backend.")
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ARK_HAS_FMOD=${ARK_HAS_FMOD}
    ARK_HAS_SOLOUD=${ARK_HAS_SOLOUD}
)

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
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -std=c++17)
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
    # Link filesystem library on GCC/Clang (if needed)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
