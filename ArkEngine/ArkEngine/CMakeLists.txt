cmake_minimum_required(VERSION 3.20)
project(ArkEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Vendored third-party headers live under <repo_root>/lib/include
set(LIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../../lib/include)

# ---------------------------------------------------------------------------
# Rework sources -- the self-contained engine foundation in src/Rework/
# ---------------------------------------------------------------------------
set(REWORK_SOURCES
    src/Rework/main.cpp
    src/Rework/Ark_basic_glfw_camera.cpp
    src/Rework/Ark_basic_mesh.cpp
    src/Rework/Ark_glfw.cpp
    src/Rework/Ark_glfw_camera_handler.cpp
    src/Rework/Ark_glm_camera.cpp
    src/Rework/Ark_math_3d.cpp
    src/Rework/Ark_new_lighting.cpp
    src/Rework/Ark_phong_renderer.cpp
    src/Rework/Ark_shadow_mapping_technique.cpp
    src/Rework/Ark_skinned_mesh.cpp
    src/Rework/Ark_skinning_technique.cpp
    src/Rework/Ark_stb_image.cpp
    src/Rework/Ark_texture.cpp
    src/Rework/Ark_Util.cpp
    src/Rework/Ark_world_transform.cpp
    src/Rework/technique.cpp
)

# meshoptimizer (used by BasicMesh and SkinnedMesh for mesh optimization)
file(GLOB MESHOPT_SOURCES ${LIB_INCLUDE_DIR}/meshoptimizer/*.cpp)

# glad loader (C source)
set(GLAD_SOURCES src/glad/glad.c)

# stb_image implementation (defines STB_IMAGE_IMPLEMENTATION)
set(STB_SOURCES ${LIB_INCLUDE_DIR}/stb_image/stb_image.cpp)

add_executable(${PROJECT_NAME}
    ${REWORK_SOURCES}
    ${GLAD_SOURCES}
    ${STB_SOURCES}
    ${MESHOPT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Rework
    ${LIB_INCLUDE_DIR}
    ${LIB_INCLUDE_DIR}/GLAD
    ${LIB_INCLUDE_DIR}/GLFW
    ${LIB_INCLUDE_DIR}/glm
    ${LIB_INCLUDE_DIR}/gli
    ${LIB_INCLUDE_DIR}/stb_image
    ${LIB_INCLUDE_DIR}/meshoptimizer
)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# Assimp
find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
endif()

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY})
endif()

# Compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX- /std:c++17)
    target_link_libraries(${PROJECT_NAME} PRIVATE filesystem)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()
