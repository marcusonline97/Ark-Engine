#pragma once

#pragma warning(push, 0)
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// Include math headers with proper defines BEFORE other headers
// This ensures math functions are available in global namespace
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <math.h>  // C-style header puts functions in global namespace
#include <cstdlib>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/hash.hpp"
#include "Math.h"
#pragma warning(pop)

// Common constants
constexpr float SMALL_NUMBER = 1e-6f;
constexpr float PI = 3.14159265358979323846f;
constexpr float ToRadian(float degrees) {
    return degrees * PI / 180.0f;
}

// Common string constants
constexpr const char* UNDEFINED_STRING = "UNDEFINED";

// Forward declarations and enums (add more as needed)
enum API {
    UNDEFINED_API = 0,
    OPENGL = 1,
    VULKAN = 2
};

enum WindowedMode {
    WINDOWED = 0,
    FULLSCREEN = 1
};

enum EngineMode {
    EDITOR,
    GAME
};

enum Weapon {
    KNIFE = 0,
    GLOCK,
    SHOTGUN,
    AKS74U,
    MP7,
    WEAPON_COUNT
};

enum WeaponAction {
    IDLE,
    FIRE,
    RELOAD,
    RELOAD_FROM_EMPTY,
    DRAW_BEGIN,
    DRAWING,
    SPAWNING,
    RELOAD_SHOTGUN_BEGIN,
    RELOAD_SHOTGUN_SINGLE_SHELL,
    RELOAD_SHOTGUN_DOUBLE_SHELL,
    RELOAD_SHOTGUN_END,
    ADS_IN,
    ADS_OUT,
    ADS_IDLE,
    ADS_FIRE
};

enum SplitscreenMode {
    NONE,
    TWO_PLAYER,
    FOUR_PLAYER,
    SPLITSCREEN_MODE_COUNT
};

enum PhysicsObjectType {
    UNDEFINED,
    GAME_OBJECT,
    GLASS,
    DOOR,
    WINDOW,
    SCENE_MESH,
    RAGDOLL_RIGID,
    CSG_OBJECT_ADDITIVE,
    CSG_OBJECT_SUBTRACTIVE,
    LIGHT
};

struct AABB {
    glm::vec3 boundsMin = glm::vec3(0);
    glm::vec3 boundsMax = glm::vec3(0);
    
    AABB() {}
    AABB(glm::vec3 min, glm::vec3 max) : boundsMin(min), boundsMax(max) {}
};

struct Triangle {
    glm::vec3 v0 = glm::vec3(0);
    glm::vec3 v1 = glm::vec3(0);
    glm::vec3 v2 = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
};

struct PhysXRayResult {
    bool hitFound = false;
    glm::vec3 hitPosition = glm::vec3(0);
    glm::vec3 surfaceNormal = glm::vec3(0);
    glm::vec3 rayDirection = glm::vec3(0);
    std::string hitObjectName = "";
    PhysicsObjectType physicsObjectType = PhysicsObjectType::UNDEFINED;
    void* hitActor = nullptr;
    void* parent = nullptr;
};

struct PhysicsObjectData {
    PhysicsObjectType type;
    void* parent;
    
    PhysicsObjectData() : type(PhysicsObjectType::UNDEFINED), parent(nullptr) {}
    PhysicsObjectData(PhysicsObjectType t, void* p) : type(t), parent(p) {}
};

struct FileInfo {
    std::string fullpath = "";
    std::string directory = "";
    std::string filename = "";
    std::string filetype = "";
    std::string materialType = "";
};

struct Transform {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 rotation = glm::vec3(0);
    glm::vec3 scale = glm::vec3(1);
    
    glm::mat4 to_mat4() const {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0));
        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0));
        glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
        return translation * rotationZ * rotationY * rotationX * scaleMat;
    }
};

// Common color constants
constexpr glm::vec3 RED = glm::vec3(1, 0, 0);
constexpr glm::vec3 GREEN = glm::vec3(0, 1, 0);
constexpr glm::vec3 BLUE = glm::vec3(0, 0, 1);
constexpr glm::vec3 YELLOW = glm::vec3(1, 1, 0);
constexpr glm::vec3 WHITE = glm::vec3(1, 1, 1);
constexpr glm::vec3 BLACK = glm::vec3(0, 0, 0);
